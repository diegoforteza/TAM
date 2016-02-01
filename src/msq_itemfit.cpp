

// includes from the plugin
#include <RcppArmadillo.h>
#include <Rcpp.h>


#ifndef BEGIN_RCPP
#define BEGIN_RCPP
#endif

#ifndef END_RCPP
#define END_RCPP
#endif

using namespace Rcpp;


// user includes


// declarations
extern "C" {
SEXP msq_itemfit( SEXP resp_, SEXP irf1_, SEXP K_, SEXP TP_, SEXP post1_, SEXP FIT_, SEXP fitIndexM_) ;
}

// definition

SEXP msq_itemfit( SEXP resp_, SEXP irf1_, SEXP K_, SEXP TP_, SEXP post1_, SEXP FIT_, SEXP fitIndexM_ ){
BEGIN_RCPP
   
       
     Rcpp::NumericMatrix resp(resp_);          
     Rcpp::NumericVector irf1(irf1_);  
     int K = as<int>(K_);  
     int TP = as<int>(TP_);  
     Rcpp::NumericMatrix post1(post1_);          
     Rcpp::NumericMatrix FIT(FIT_);   
     Rcpp::NumericMatrix fitIndexM(fitIndexM_);  
       
       
     int N=resp.nrow();  
     int I=resp.ncol();  
     int FF = FIT.nrow();  
       
       
     Rcpp::NumericVector probs_categ(N*K*TP*I);   
     // Rcpp::NumericVector pred(N*TP*I);  
     arma::cube pred(N,TP,I);  
     arma::cube var1(N,TP,I);  
     arma::cube resid1(N,TP,I);  
     arma::cube sresid1(N,TP,I);  
       
     // create output table  
     Rcpp::NumericMatrix dfr_fit(FF,4);  
       
       
     ////********************************************************  
     ///   model predictions  
     ///*********************************************************  
       
     double p1=0;  
     double v1=0;  
       
     for (int ii=0;ii<I;ii++){  
     for (int nn=0;nn<N;nn++){  
       if ( ! R_IsNA( resp(nn,ii) ) ){  
     	for (int tt=0;tt<TP;tt++){ // begin tt  
     	       v1 = 0 ;  
     		for (int kk=0;kk<K;kk++){ // begin kk  
     		    p1 = irf1[ ii + kk*I + tt*I*K ] ;  
     		    probs_categ[ nn + kk*N + tt*N*K + ii*N*K*TP ] = p1 ;  
     		    v1 += kk * p1 ;  
     				} // end kk  
     	       pred(nn,tt,ii) = v1 ;   			  
     	       v1 = 0 ;  
     		for (int kk=0;kk<K;kk++){ // begin kk  
     		    p1 = irf1[ ii + kk*I + tt*I*K ] ;  
     		    v1 += pow( kk - pred(nn,tt,ii) , 2.0 ) * p1 ;  
     				} // end kk         
     	  
     	       var1(nn,tt,ii) = v1 ;    
     	       // residuals  
     	       resid1(nn,tt,ii) = ( resp( nn , ii ) - pred(nn,tt,ii) ) ;  
     	       sresid1(nn,tt,ii) = resid1(nn,tt,ii) / sqrt( var1(nn,tt,ii) ) ;   
     	       	         
     		 } // end tt 		 		 		   
     		   
     	    }  
       if ( R_IsNA( resp(nn,ii) ) ){  
         for (int tt=0;tt<TP;tt++){  
       	pred(nn,tt,ii) = NA_REAL ;  
       	resid1(nn,tt,ii) = NA_REAL ;  
       	sresid1(nn,tt,ii) = NA_REAL ;  	  
       			     }  
       			}  
     	       } // end nn  
     	     } // end ii  
       
     	       
     ////********************************************************  
     ///   calculation of fit statistics  
     ///*********************************************************	       
       
     Rcpp::NumericVector fit0(N);  
     Rcpp::NumericVector fit1(N);  
     Rcpp::NumericVector fit_temp(N);  
     Rcpp::NumericVector fit_temp2(N);  
     Rcpp::NumericMatrix kurt_ii(N,TP);  
     Rcpp::NumericMatrix v0(N,TP);  
     Rcpp::NumericVector vec1(N);  
     Rcpp::NumericVector vec0(N);  
       
       
     double N1=0 ;  
     double term1=0;  
     double term2=0;  
     double ot=1/(3.0) ;  
       
       
     for ( int ff=0; ff < FF ; ff++ ){   
       
     //**********************************  
     //  OUTFIT Calculation  
       
     N1=0;  
     for (int nn=0;nn<N;nn++){  // beg nn	  
         fit0[nn] = 0 ;		  
         for (int hh = FIT(ff,1) ; hh < FIT(ff,2) + 1 ; hh++){  // beg ii  
         	int ii = fitIndexM(hh,0) ;      
     	if ( ! R_IsNA( resp(nn,ii) ) ){  // beg NA resp(nn,ii)  
     		fit_temp[nn] = 0 ;  
     		N1 += 1 ;  
     		for (int tt=0;tt<TP;tt++){  
     		    fit_temp[nn] += post1(nn,tt) * pow( sresid1(nn,tt,ii) , 2.0 ) ;  
     					}  
     		fit0[nn] += fit_temp[nn] ;			  
     			}  // end missing resp(nn,ii)  
                  } // end ii			  
     	} // end nn	  
         		  
     term1=0;    		  
     for (int nn=0;nn<N;nn++){  
     	term1 += fit0[nn] ;  
         		}  
     dfr_fit(ff,0) = term1 / N1 ;    		  
         		  
       
     //**********************************  
     //  OUTFIT t Statistic  
       
       
       
     double qi=0;  
       
     for (int nn=0;nn<N;nn++){  // beg nn    
       vec1[nn] = 0 ;     
       for (int tt=0;tt<TP;tt++){ kurt_ii(nn,tt) = 0 ; }  
       for (int hh=FIT(ff,1);hh<FIT(ff,2)+1;hh++){ // beg ii  
         int ii = fitIndexM(hh,0) ;  
         if ( ! R_IsNA( resp(nn,ii) ) ){  
     	  vec0[nn]=0;	  
     	  for (int tt=0;tt<TP;tt++){  // beg tt  
     	     kurt_ii(nn,tt) = 0 ; 	    
     	     for (int kk=0;kk<K;kk++){  // beg kk  
     		kurt_ii(nn,tt) += irf1[ ii + kk*I + tt*I*K ] * pow( kk - pred(nn,tt,ii) , 4.0 ) ;  
     				}  // end kk  
     	   v0(nn,tt) = kurt_ii(nn,tt) / pow( var1( nn , tt, ii ) , 2.0) ;  
     	   vec0[nn] += post1(nn,tt) * v0(nn,tt) ;  
     				}  // end tt  
                vec1[nn] += vec0[nn];   			  
                		}  // end missing resp(nn,ii)  
                	} // end ii		  
     	}  // end nn  
       
     qi=0;  
     for (int nn=0;nn<N;nn++){  qi += vec1[nn] ; }  
     qi = qi / pow(N1 , 2.0 ) - 1 / N1 ;  
     //  dfr[ff,"Outfit_t"] <- ( fit0^(1/3)-1 )* 3 / sqrt(qi) + sqrt(qi) / 3   
     dfr_fit(ff,1) = ( pow( dfr_fit(ff,0) , ot ) - 1 ) * 3 / sqrt( qi ) + sqrt( qi ) / 3 ;  
       
       
       
       
       
     //**********************************  
     //  INFIT Calculation  
       
     N1=0;  
     for (int nn=0;nn<N;nn++){  // beg nn	  
         fit0[nn] = 0 ;	  
         fit1[nn] = 0 ;  
         for (int hh = FIT(ff,1) ; hh < FIT(ff,2) + 1 ; hh++){  // beg ii	  
         	int ii = fitIndexM(hh,0) ;  
     	if ( ! R_IsNA( resp(nn,ii) ) ){  // beg NA resp(nn,ii)  
     		fit_temp[nn] = 0 ;  
     		fit_temp2[nn] = 0 ;  
     		N1 += 1 ;  
     		for (int tt=0;tt<TP;tt++){  
     		    fit_temp[nn] += post1(nn,tt) * pow( sresid1(nn,tt,ii) , 2.0 )*  
     		    		var1(nn,tt,ii);  
     		    fit_temp2[nn] += post1(nn,tt)*var1(nn,tt,ii) ;  
     					}  
     		fit0[nn] += fit_temp[nn] ;  
     		fit1[nn] += fit_temp2[nn];  
     			}  // end missing resp(nn,ii)  
                  } // end ii			  
     	} // end nn	  
         		  
     term1=0;    		  
     term2=0;  
     for (int nn=0;nn<N;nn++){  
     	term1 += fit0[nn];  
     	term2 += fit1[nn];  
         		}  
     dfr_fit(ff,2) = term1 / term2 ;    		  
         		  
       
     //**********************************  
     //  INFIT t Statistic  
       
       
       
     for (int nn=0;nn<N;nn++){  // beg nn    
       vec1[nn] = 0 ;     
       for (int tt=0;tt<TP;tt++){ kurt_ii(nn,tt) = 0 ; }  
       for (int hh=FIT(ff,1);hh<FIT(ff,2)+1;hh++){ // beg ii  
         int ii = fitIndexM(hh,0) ;  
         if ( ! R_IsNA( resp(nn,ii) ) ){  
     	  vec0[nn]=0;	  
     	  for (int tt=0;tt<TP;tt++){  // beg tt  
     	     kurt_ii(nn,tt) = 0 ; 	    
     	     for (int kk=0;kk<K;kk++){  // beg kk  
     		kurt_ii(nn,tt) += irf1[ ii + kk*I + tt*I*K ] * pow( kk - pred(nn,tt,ii) , 4.0 ) ;  
     				}  // end kk  
     	   v0(nn,tt) = ( kurt_ii(nn,tt) - pow( var1( nn , tt, ii ) , 2.0) ) ;  
     	   vec0[nn] += post1(nn,tt) * v0(nn,tt) ;  
     				}  // end tt  
                vec1[nn] += vec0[nn];   			  
                		}  // end missing resp(nn,ii)  
                	} // end ii		  
     	}  // end nn  
       
     qi=0;  
     for (int nn=0;nn<N;nn++){  qi += vec1[nn] ; }  
     qi = qi / pow( term2 , 2.0 ) ;  
     //  dfr[ff,"Outfit_t"] <- ( fit0^(1/3)-1 )* 3 / sqrt(qi) + sqrt(qi) / 3   
     dfr_fit(ff,3) = ( pow( dfr_fit(ff,2) , ot ) - 1 ) * 3 / sqrt( qi ) + sqrt( qi ) / 3 ;  
       
     }  
       
       
     //	Rcpp::Rcout << "Ntot=" <<  Ntot <<  std::flush << std::endl ;  
         		  
         		  
     //*************************************************      
     // OUTPUT              
               
     return Rcpp::List::create(   
            Rcpp::_["dfr_fit"] = dfr_fit  
         ) ;    
       
     // maximal list length is 20!  
       
       
     // Rcpp::Rcout << "tmp1 " <<  tmp1 <<  std::flush << std::endl ;  
       
       
     
END_RCPP
}



