#include "mmc.hpp"

mmc::mmc(int wchoice)
{
  _debug = false;
  if( _debug) cout << "MMC: mmc instance created" << endl;
  if( _debug) cout << __PRETTY_FUNCTION__ << endl;

  cout << endl;
  cout << "###############################################"<< endl;
  cout << "MMC tag: mmc-00-01-10, 10/07/2012              "<< endl;
  cout << "Contact: martin.flechl@cern.ch                 "<< endl;
  cout << "         michael.pitt@cern.ch                  "<< endl;
  cout << "         ohad.silbert@cern.ch                  "<< endl;
  cout << "         Julian.Maluck@physik.uni-freiburg.de  "<< endl;
  cout << "###############################################"<< endl;

  cout << endl;

  // set pt cut:
  f_cut=0.15;	// f = lep_PTCUT*cosh(eta_lep) / E_tau
  
  // --------------------- Phasespace scan size ---------------------- // 
  m_mma_limlow=0.05;		// minimum missing mass value
  m_mma_limhigh=1.65;		// maximum missing mass value
  theta_star_max = 0.8;		// cos(theta*) max value
  m_mma_n_6D=3;				// missing mass scan points in 6D scan
  m_costheta_6D = 5;		// cos(theta*) scan points in 6D scan
  MET_scan_points_Phi=25;	// missing energy scan:		Z   = exp( -(Ex^2+Ey^2)/ 2sigma^2)
  MET_scan_points_Z=25;		//							phi = atan(Ey/Ex)
  m_mma_n_4D=7;				// missing mass scan points in 4D scan
  m_costheta_4D = 40;		// cos(theta*) scan points in 4D scan
  m_met_limsigma=3.; 		// Range of MET scan (in sigma)
// ----------------------------------------------------------------- // 
  
// Initialize variables
  m_mz_peak=-1;
  m_mz_mean=-1;
  m_mz_maxprob=-1;
  m_exectime=-1;

  // used in older implementations (00-01-08 and older)
  m_alpha=0.7;
  m_noise=0.0;

  wchoice=2;

  m_phi_n=20; //was: 25
  m_theta_n=20;
  m_met_n=25;
  m_mma_n=10;
  m_costheta = 30;

  m_phi_lim=0.2; //*(100/tau_p) : was: 0.12
  m_theta_lim=0.1;

  m_met_lim=10;


  m_met_scanmode=2;
  
  m_met_s=1;


  m_weight=0;
  m_mm_weight=1; //was: 0

//  this->GetMDistribution();
//  this->SetAngleWeight(wchoice);

}


float mmc::Scan6d(const TLorentzVector _lep1, const TLorentzVector _lep2, const float _metx, const float _mety, const float _sumet)
{

  if( _debug) cout << "################################" << endl;
  if( _debug) cout << __PRETTY_FUNCTION__ << endl;
  if( _debug) cout << "MMC: Starting 6D scan" << endl;
  if( _debug) cout << "Using alpha=" << this->m_alpha << endl;



  clock_t c_t0=clock(),c_t1;

  if ( !( m_weight==0 || m_weight==1 || m_weight==2  ||m_weight==3) ){ cout << "Not a valid weight choice! Allowed are 0-3 Exiting..." << endl; return -1; }

  const float ntheta3d_lim=0.07;
  const float _SMALL_GEV_=1E-5;
  const float _LARGE_GEV_=1E4;

  int have_warned=0;

  m_mz_peak=-1;
  m_mz_mean=-1;
  m_mz_maxprob=-1;

  float mz_maxprob=-1;
  float maxprob=-1;
  float weightsum=0;
  float mzweightedsum=0;
  
  TH1F* h_mz_find_peak = new TH1F("h_mz_find_peak", "h_mz_find_peak", 500, 0., 500.);

  const float mex_sigma = m_alpha * sqrt(_sumet);
  const float mey_sigma = m_alpha * sqrt(_sumet);

  //to speed things up - use floats instead of TLV
  float lep1__Phi=_lep1.Phi();
  float lep1__Theta=_lep1.Theta();
  float lep1__M=_lep1.M();
  float lep1__P=_lep1.P();
  float lep1__PZ=_lep1.Pz();

  float sin__lep1_Phi = sin(lep1__Phi);
  float cos__lep1_Phi = cos(lep1__Phi);
  float sin__lep1_Theta = sin(lep1__Theta);
  //  float cos__lep1_Theta = cos(lep1__Theta);

  float lep2__Phi=_lep2.Phi();
  float lep2__Theta=_lep2.Theta();
  float lep2__M=_lep2.M();
  float lep2__P=_lep2.P();
  float lep2__PZ=_lep2.Pz();

  float sin__lep2_Phi = sin(lep2__Phi);
  float cos__lep2_Phi = cos(lep2__Phi);
  float sin__lep2_Theta = sin(lep2__Theta);
  //  float cos__lep2_Theta = cos(lep2__Theta);
    
  TLorentzVector tau1_gemplus;
  TLorentzVector tau2_gemplus;
  TLorentzVector nu1_gemplus;
  TLorentzVector nu2_gemplus;
  TLorentzVector z_gemplus; 
  TLorentzVector tau1_gemminus;
  TLorentzVector tau2_gemminus;
  TLorentzVector nu1_gemminus;
  TLorentzVector nu2_gemminus;
  TLorentzVector z_gemminus; 
  TLorentzVector z_gemplusminus; 
  TLorentzVector z_gemminusplus;
      
  //  const int c_50M=50*1000*1000;
  //  vector<float> v_zm_best(c_50M);
  //  vector<float> v_weight(c_50M);

  /*
  vector<float> v_zm_best;
  vector<float> v_weight;
  */
    
  /*  
  float sinTheta1plus;
  float sinTheta1minus;
  float sinTheta2plus;
  float sinTheta2minus;
  float p1plus;
  float p2plus;
  float p1minus;
  float p2minus;
  */
  
  float alpha1;
  float alpha2;
  float beta1;
  float beta2;
  float x1_1;
  float x1_2;
  float x2_1;
  float x2_2;

  /*
  float r1;
  float r2;
  float t1;
  float t2;
  float y1;
  float y2;
  float z1;
  float z2;
  float u1;
  float u2;
  float sin2plus1;
  float sin2plus2;
  float sin2minus1;
  float sin2minus2;
  float theta2plus1;
  float theta2plus2;
  float theta2minus1;
  float theta2minus2;
  float theta1plus1;
  float theta1plus2;
  float theta1minus1;
  float theta1minus2;
  */

  float weight[4]={0};
  float weight_angle[4]={0};

  float c1;
  float c2; 
  
  //  float v_theta1_h[4];
  //  float v_theta2_h[4];
  //  float v_theta1[2];
  //  float v_theta2[2];

  //  int ctr1;
  //  int ctr2;
  
  //  float cosdeltaTheta1;
  //  float cosdeltaTheta2;
  //  float mtau1;
  //  float mtau2;
  
  float deltaR[4]={0};
  //  float deltaTheta[4]={0};
  float m[4]={0};
  float ntheta3d[4]={0};
  
  float weight_met;
  float weight_mex;
  float weight_mey;

  float sin__nu1phi;      
  float sin__nu2phi;      
  float cos__nu1phi;      
  float cos__nu2phi;      

  //const wrt loop
  beta2 = lep2__P*lep2__P + lep2__M*lep2__M;
  beta1 = lep1__P*lep1__P + lep1__M*lep1__M;

  float step_phi=2*m_phi_lim/(m_phi_n-1);
  //  float step_met=2*m_met_lim/(m_met_n-1);
  float step_mma=(m_mma_limhigh-m_mma_limlow)/(m_mma_n-1);

  float step_metx=0; float step_mety=0;
  float lim_metx=0; float lim_mety=0;
  float ncomb=m_mma_n*m_mma_n*m_phi_n*m_phi_n;
  if (m_met_scanmode==0){
    step_metx=m_met_s;
    step_mety=m_met_s;
    lim_metx=m_met_limsigma*mex_sigma;
    lim_mety=m_met_limsigma*mey_sigma;
    ncomb*= (m_met_limsigma*mex_sigma/m_met_s)*(m_met_limsigma*mey_sigma/m_met_s);
  } else if(m_met_scanmode==1){
    step_metx=2*m_met_lim/(m_met_n-1);
    step_mety=step_metx;
    lim_metx=m_met_lim;
    lim_mety=m_met_lim;
    ncomb*=m_met_n*m_met_n;
  } else{
    lim_metx=m_met_limsigma*mex_sigma;
    lim_mety=m_met_limsigma*mey_sigma;
    step_metx=2*lim_metx/(m_met_n-1);
    step_mety=2*lim_mety/(m_met_n-1);
    ncomb*=m_met_n*m_met_n;
  }

  if( _debug){ cout << "Trying " << ncomb << " scan points" << endl;
               cout << "MMC parameters: " << endl;
               cout << "m_met_s=" << m_met_s << endl;
               cout << "m_met_limsigma=" << m_met_limsigma << endl;
               cout << "m_met_lim=" << m_met_lim << endl;
               cout << "m_met_scanmode=" << m_met_scanmode << endl;
               cout << "m_met_n=" << m_met_n << endl;
               cout << "m_met_lim=" << m_met_lim << endl;
               cout << "m_mma_n=" << m_mma_n << endl;
               cout << "m_mma_limlow=" << m_mma_limlow << endl;
               cout << "m_mma_limhigh=" << m_mma_limhigh << endl;
               cout << "m_phi_n=" << m_phi_n << endl;
               cout << "m_phi_lim=" << m_phi_lim << endl; }


  if( _debug){ cout << "MEx step size: " << step_metx << ", MEy step size: " << step_mety << ", Scan x: " << _metx-lim_metx << "-" << _metx+lim_metx
                    << ", Scan y: " <<_mety-lim_mety << "-" << _mety+lim_mety << ", sumEt: " << _sumet << endl; }
  //Loop 1
  for( float nu1phi=lep1__Phi-m_phi_lim; nu1phi<(lep1__Phi+m_phi_lim); nu1phi+=step_phi){
    if (_debug) cout << "1 - phi1=" << nu1phi << endl;  
    //    if( fabs(nu1phi)>TMath::Pi()) continue;
    sin__nu1phi=sin(nu1phi);
    cos__nu1phi=cos(nu1phi);

    x1_1 = cos__nu1phi*sin__lep1_Theta*cos__lep1_Phi;
    x2_1 = sin__nu1phi*sin__lep1_Theta*sin__lep1_Phi;

    //Loop 2
    for( float nu2phi=lep2__Phi-m_phi_lim; nu2phi<(lep2__Phi+m_phi_lim); nu2phi+=step_phi){
      //if (_debug) cout << "2 - phi2=" << nu2phi << endl;  
      // if( fabs(nu2phi)>TMath::Pi()) continue;
      sin__nu2phi=sin(nu2phi);
      cos__nu2phi=cos(nu2phi);

      x1_2 = cos__nu2phi*sin__lep2_Theta*cos__lep2_Phi;
      x2_2 = sin__nu2phi*sin__lep2_Theta*sin__lep2_Phi;
          
      //Loop 3
      for( float etx=_metx-lim_metx; etx<_metx+lim_metx; etx+=step_metx){
         weight_mex = GetMETValue(_metx-etx, mex_sigma);

        //Loop 4
        for( float ety=_mety-lim_mety; ety<_mety+lim_mety; ety+=step_mety){
          c2 = (etx*sin__nu1phi - ety*cos__nu1phi)		// is actually signed pt_mis2
               / (cos__nu2phi*sin__nu1phi - sin__nu2phi*cos__nu1phi);
          c1 = (etx*sin__nu2phi - ety*cos__nu2phi)		// is actually signed pt_mis1
               / (cos__nu1phi*sin__nu2phi - sin__nu1phi*cos__nu2phi);

          if ( (c1 > _LARGE_GEV_) || (c2 > _LARGE_GEV_) ){ 
	    if( _debug && !(have_warned) ) cout << "WARNING --- MMC: (c1 || c2)>" << _LARGE_GEV_ << endl;
            have_warned=1;
            continue;
	  }
          if ( (fabs(c1) < _SMALL_GEV_) || (fabs(c2) < _SMALL_GEV_) ){ 
	    if( _debug && !(have_warned) ) cout << "WARNING --- MMC: (|c1| || |c2|)=(" << fabs(c1) << " || " << fabs(c2) << ") <" <<_SMALL_GEV_ << endl;
            have_warned=1;
            continue;
	  }


          weight_mey = GetMETValue(_mety-ety, mey_sigma);
                               
          //Loop 5
          for( float nu1m=m_mma_limlow; nu1m<m_mma_limhigh; nu1m+=step_mma){	// scan in m1
            alpha1 = (1.777*1.777 - nu1m*nu1m - lep1__M*lep1__M) / 2;

	    /*
            r1 = c1*lep1__P*alpha1 / (alpha1*alpha1 - beta1*nu1m*nu1m);
            t1 = c1*c1 / (alpha1*alpha1 - beta1*nu1m*nu1m);
            y1 = 1 	 + 2*r1*x1_1   	 + 2*r1*x2_1   	 + t1*lep1__P*lep1__P*x1_1*x1_1
                  	 + t1*lep1__P*lep1__P*x2_1*x2_1
                  	 - t1*lep1__P*lep1__P*cos__lep1_Theta*cos__lep1_Theta
                  	 + t1*lep1__P*lep1__P*2*x1_1*x2_1;
            z1 = 2*r1*cos__lep1_Theta
                  	 + 2*t1*lep1__P*lep1__P*x1_1*cos__lep1_Theta
                   	 + 2*t1*lep1__P*lep1__P*x2_1*cos__lep1_Theta;
            u1 = t1*lep1__P*lep1__P*cos__lep1_Theta*cos__lep1_Theta
               	 - t1*beta1;
            if ( (2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1)*(2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1) - u1*u1 / (y1*y1 + z1*z1) < 0 ) continue;

            // from the 'quadratic' equation we Get 2 solutions for each sin^2 and each neutrinosystem  
            sin2plus1 = - (2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1)
              		    + sqrt((2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1)*(2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1)
			    - u1*u1 / (y1*y1 + z1*z1));				
            sin2minus1= - (2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1)
             		    - sqrt((2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1)*(2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1)
                  				    - u1*u1 / (y1*y1 + z1*z1));		
            // only the positive values of sqrt(sin^2) physically meaningful
            sinTheta1plus = sqrt(sin2plus1);
            sinTheta1minus = sqrt(sin2minus1);

            // for each sin(theta) I get 2 solutions for theta
            theta1plus1 = asin(sinTheta1plus);
            theta1minus1 = asin(sinTheta1minus);
            theta1plus2 = TMath::Pi() - asin(sinTheta1plus);
            theta1minus2 = TMath::Pi() - asin(sinTheta1minus);

            v_theta1_h[0]=theta1plus1;
            v_theta1_h[1]=theta1plus2;
            v_theta1_h[2]=theta1minus1;
            v_theta1_h[3]=theta1minus2;

            // testing the theta-solutions, by putting each solution in the equation for Mtau^2 and check if the solutions really solve the equation              
            ctr1=0;
            for( size_t i=0; i<4; i++){
              cosdeltaTheta1 = sin(v_theta1_h[i])*cos__nu1phi*sin__lep1_Theta*cos__lep1_Phi 
                  		   + sin(v_theta1_h[i])*sin__nu1phi*sin__lep1_Theta*sin__lep1_Phi 
                  		   + cos(v_theta1_h[i])*cos__lep1_Theta;
              mtau1 = nu1m*nu1m     + lep1__M*lep1__M + 2*sqrt( lep1__P*lep1__P 
                         + lep1__M*lep1__M )*sqrt( c1/sin(v_theta1_h[i])*c1/sin(v_theta1_h[i]) + nu1m*nu1m ) 
                  	 - 2*lep1__P*c1/sin(v_theta1_h[i])*cosdeltaTheta1; 
	      //              if( (mtau1 > 3.15772 && mtau1 < 3.15774)){
              if( (mtau1 > 3.15 && mtau1 < 3.17)){
                if (ctr1<2) v_theta1[ctr1]=v_theta1_h[i];
                ctr1++;
              }
	    }
	    //            cout << "XX " << ctr1 << endl;
            // only 2 theta-solutions solve tue Mtau^2 equation for each neutrinosystem
            if( ctr1 != 2 ) continue;

            // values for the scalar momentum of the neutrinosystems
            p1plus = c1 / sin(v_theta1[0]);
            p1minus = c1 / sin(v_theta1[1]); 
	    */

	    float b1 = c1*lep1__P*(x1_1+x2_1) + alpha1; 
	    float del1 = b1*b1 - (c1*c1+nu1m*nu1m)*(beta1 - lep1__PZ*lep1__PZ);
	    if (del1<0.) continue;

            //Loop 6
            for( float nu2m=m_mma_limlow; nu2m<m_mma_limhigh; nu2m+=step_mma){       // scan in m2
              // just a few constants, that should make the whole thing easier...
              alpha2 = (1.777*1.777 - nu2m*nu2m - lep2__M*lep2__M) / 2;

	      float b2 = c2*lep2__P*(x1_2+x2_2) + alpha2; 
	      float del2 = b2*b2 - (c2*c2+nu2m*nu2m)*(beta2 - lep2__PZ*lep2__PZ);
	      if (del2<0.) continue;



              // filling the calculated neutrinosystem-four-vectors
	      //          nu1_gemplus.SetPtEtaPhiM(c1, -log(tan(v_theta1[0]/2)), nu1phi, nu1m);
              nu1_gemplus.SetXYZM(c1*cos__nu1phi, c1*sin__nu1phi, (b1*lep1__PZ+sqrt(beta1*del1))/(beta1 - lep1__PZ*lep1__PZ), nu1m);
              tau1_gemplus = nu1_gemplus + _lep1;
	      //            nu1_gemminus.SetPtEtaPhiM(c1, -log(tan(v_theta1[1]/2)), nu1phi, nu1m);
  	      nu1_gemminus.SetXYZM(c1*cos__nu1phi, c1*sin__nu1phi, (b1*lep1__PZ-sqrt(beta1*del1))/(beta1 - lep1__PZ*lep1__PZ), nu1m);
              tau1_gemminus = nu1_gemminus + _lep1;

              // calculating DeltaR and thus the probability for each solution
	      if (m_weight==0){
                cout << "Error: This weight is currently not supported!" << endl;
		/*
                deltaTheta[0] =  fabs(TVector2::Phi_mpi_pi(v_theta1[0]-lep1__Theta));
                deltaTheta[1] = fabs(TVector2::Phi_mpi_pi(v_theta1[1]-lep1__Theta));
                weight_angle[0]=GetDeltaThetaValue(deltaTheta[0], tau1_gemplus.P());
                weight_angle[1]=GetDeltaThetaValue(deltaTheta[1], tau1_gemminus.P());
		*/
	      } else if (m_weight==1){
                deltaR[0] = _lep1.DeltaR(nu1_gemplus);
                deltaR[1] = _lep1.DeltaR(nu1_gemminus);
                weight_angle[0]=GetDeltaRValue(deltaR[0], tau1_gemplus.P());
                weight_angle[1]=GetDeltaRValue(deltaR[1], tau1_gemminus.P());
	      } else if (m_weight==2){
   	        weight_angle[0]=GetAnaWeight_TauMiss(tau1_gemplus, nu1_gemplus, ntheta3d[0]);
   	        weight_angle[1]=GetAnaWeight_TauMiss(tau1_gemminus, nu1_gemminus, ntheta3d[1]);
                if( (ntheta3d[0]>ntheta3d_lim) && (ntheta3d[1]>ntheta3d_lim) ) continue;
	      }    

              m[0] = nu1_gemplus.M();
              m[1] = nu1_gemminus.M();

  	      //            deltaPhi[0] = _lep1.DeltaPhi(nu1_gemplus);
	      //            deltaPhi[1] = _lep1.DeltaPhi(nu1_gemminus);


	    /*
               r2 = c2*lep2__P*alpha2 / (alpha2*alpha2 - beta2*nu2m*nu2m);
               t2 = c2*c2 / (alpha2*alpha2 - beta2*nu2m*nu2m);
    
               // with the next constants quadratic equation for sin(theta_mis) reads: 0=y sin^2 +z sin sqrt(1-sin^2) +u
               y2 = 1 	 + 2*r2*x1_2   	 + 2*r2*x2_2   	 + t2*lep2__P*lep2__P*x1_2*x1_2
                  	 + t2*lep2__P*lep2__P*x2_2*x2_2
                  	 - t2*lep2__P*lep2__P*cos__lep2_Theta*cos__lep2_Theta
                  	 + t2*lep2__P*lep2__P*2*x1_2*x2_2;
               z2 = 2*r2*cos__lep2_Theta
                  	 + 2*t2*lep2__P*lep2__P*x1_2*cos__lep2_Theta
                   	 + 2*t2*lep2__P*lep2__P*x2_2*cos__lep2_Theta;
               u2 = t2*lep2__P*lep2__P*cos__lep2_Theta*cos__lep2_Theta
                  	 - t2*beta2;
    
               if ( (2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2)*(2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2) - u2*u2 / (y2*y2 + z2*z2) < 0 ) continue;
               
	       //               if( (2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2)*(2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2) - u2*u2 / (y2*y2 + z2*z2) < 0 || (2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1)*(2*y1*u1 - z1*z1) / (2*y1*y1 + 2*z1*z1) - u1*u1 / (y1*y1 + z1*z1) < 0) continue;		// to be sure, that the squareroot is bigger than 0
    
               // from the 'quadratic' equation we get 2 solutions for each sin^2 and each neutrinosystem  
               sin2plus2 = - (2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2)
              		    + sqrt((2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2)*(2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2)
 		            - u2*u2 / (y2*y2 + z2*z2));
               sin2minus2= - (2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2)
       			    - sqrt((2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2)*(2*y2*u2 - z2*z2) / (2*y2*y2 + 2*z2*z2)
                  				    - u2*u2 / (y2*y2 + z2*z2));
    
               // only the positive values of sqrt(sin^2) physically meaningful
               sinTheta2plus = sqrt(sin2plus2);
               sinTheta2minus = sqrt(sin2minus2);
                  
               // for each sin(theta) i get 2 solutions for theta
               theta2plus1 = asin(sinTheta2plus);
               theta2minus1 = asin(sinTheta2minus);
               theta2plus2 = TMath::Pi() - asin(sinTheta2plus);
               theta2minus2 = TMath::Pi() - asin(sinTheta2minus);

               v_theta2_h[0]=theta2plus1;
               v_theta2_h[1]=theta2plus2;
               v_theta2_h[2]=theta2minus1;
               v_theta2_h[3]=theta2minus2;
                  
               ctr2=0;
               for( size_t i=0; i<4; i++){
                 cosdeltaTheta2 = sin(v_theta2_h[i])*cos__nu2phi*sin__lep2_Theta*cos__lep2_Phi 
       				  + sin(v_theta2_h[i])*sin__nu2phi*sin__lep2_Theta*sin__lep2_Phi 
       				  + cos(v_theta2_h[i])*cos__lep2_Theta;
                 mtau2 = nu2m*nu2m + lep2__M*lep2__M + 2*sqrt( lep2__P*lep2__P 
                          + lep2__M*lep2__M )*sqrt( c2/sin(v_theta2_h[i])*c2/sin(v_theta2_h[i]) + nu2m*nu2m ) 
                  	  - 2*lep2__P*c2/sin(v_theta2_h[i])*cosdeltaTheta2; 
		 //                 if( (mtau2 > 3.15772 && mtau2 < 3.15774)){
                 if( (mtau2 > 3.15 && mtau2 < 3.17)){
                   if (ctr2<2) v_theta2[ctr2]=v_theta2_h[i];
                    ctr2++;
                 }
               }
               // only 2 theta-solutions solve tue Mtau^2 equation for each neutrinosystem
               if( ctr2 != 2 ) continue;
    
               // values for the scalar momentum of the neutrinosystems
               p2plus = c2 / sin(v_theta2[0]);
               p2minus = c2 / sin(v_theta2[1]);
	    */

            // filling the calculated neutrinosystem-four-vectors
	      //               nu2_gemplus.SetPtEtaPhiM(c2, -log(tan(v_theta2[0]/2)), nu2phi, nu2m);
              nu2_gemplus.SetXYZM(c2*cos__nu2phi, c2*sin__nu2phi, (b2*lep2__PZ + sqrt(beta2*del2))/(beta2 - lep2__PZ*lep2__PZ), nu2m);
              tau2_gemplus = nu2_gemplus + _lep2;
	      //               nu2_gemminus.SetPtEtaPhiM(c2, -log(tan(v_theta2[1]/2)), nu2phi, nu2m);
	      nu2_gemminus.SetXYZM(c2*cos__nu2phi, c2*sin__nu2phi, (b2*lep2__PZ - sqrt(beta2*del2))/(beta2 - lep2__PZ*lep2__PZ), nu2m);
              tau2_gemminus = nu2_gemminus + _lep2;
                
              // combining the four-vectors of the taus to 4 diffrent solutions for the Z-four-vector
              z_gemplus = tau1_gemplus + tau2_gemplus;
              z_gemminus = tau1_gemminus + tau2_gemminus;
              z_gemplusminus = tau1_gemplus + tau2_gemminus;
              z_gemminusplus = tau1_gemminus + tau2_gemplus;
    
              // calculating DeltaR and thus the probability for each solution
              m[2] = nu2_gemplus.M();
              m[3] = nu2_gemminus.M();
                  
	       //               deltaPhi[2] = _lep2.DeltaPhi(nu2_gemplus);
	       //               deltaPhi[3] = _lep2.DeltaPhi(nu2_gemminus);

               weight_met=weight_mex*weight_mey;
               if (m_weight==0){
                 cout << "Error: This weight is currently not supported!" << endl;
		 /*
                 deltaTheta[2] =  fabs(TVector2::Phi_mpi_pi(v_theta2[0]-lep2__Theta));
		 deltaTheta[3] = fabs(TVector2::Phi_mpi_pi(v_theta2[1]-lep2__Theta));
                 weight_angle[2]=GetDeltaThetaValue(deltaTheta[2], tau2_gemplus.P());
                 weight_angle[3]=GetDeltaThetaValue(deltaTheta[3], tau2_gemminus.P());
		 */
	       } else if (m_weight==1){
                 deltaR[2] = _lep2.DeltaR(nu2_gemplus);
                 deltaR[3] = _lep2.DeltaR(nu2_gemminus);
                 weight_angle[2]=GetDeltaRValue(deltaR[2], tau2_gemplus.P());
                 weight_angle[3]=GetDeltaRValue(deltaR[3], tau2_gemminus.P());
	       } else if (m_weight==2){
   	         weight_angle[2]=GetAnaWeight_TauMiss(tau2_gemplus, nu2_gemplus, ntheta3d[2]);
   	         weight_angle[3]=GetAnaWeight_TauMiss(tau2_gemminus, nu2_gemminus, ntheta3d[3]);
                 if( (ntheta3d[2]>ntheta3d_lim) && (ntheta3d[3]>ntheta3d_lim) ) continue;
	       }    

            if(m_weight<3){
			   weight[0] = weight_angle[0]*weight_angle[2] * weight_met;
               if (m_mm_weight) weight[0] *= GetMValue(m[0])* GetMValue(m[2]);
               weight[1] = weight_angle[1]*weight_angle[3] * weight_met;
               if (m_mm_weight) weight[1] *= GetMValue(m[1])* GetMValue(m[3]);
               weight[2] = weight_angle[0]*weight_angle[3] * weight_met;
               if (m_mm_weight) weight[2] *= GetMValue(m[0])* GetMValue(m[3]);
               weight[3] = weight_angle[1]*weight_angle[2] * weight_met;
               if (m_mm_weight) weight[3] *= GetMValue(m[1])* GetMValue(m[2]);
			}
			else if(3==m_weight){
               weight[0] = GetJacobian(_lep1, _lep2, nu1_gemplus, nu2_gemplus, etx , ety) * weight_met;
               weight[1] = GetJacobian(_lep1, _lep2, nu1_gemminus, nu2_gemminus, etx , ety) * weight_met;
               weight[2] = GetJacobian(_lep1, _lep2, nu1_gemplus, nu2_gemminus, etx , ety) * weight_met;
               weight[3] = GetJacobian(_lep1, _lep2, nu1_gemminus, nu2_gemplus, etx , ety) * weight_met;
			}			

               mzweightedsum+=z_gemplus.M()*weight[0]+z_gemminus.M()*weight[1]+z_gemplusminus.M()*weight[2]+z_gemminusplus.M()*weight[3];
               weightsum+=weight[0]+weight[1]+weight[2]+weight[3];
               if (maxprob<weight[0]){ maxprob=weight[0]; mz_maxprob=z_gemplus.M(); }
               if (maxprob<weight[1]){ maxprob=weight[1]; mz_maxprob=z_gemminus.M(); }
               if (maxprob<weight[2]){ maxprob=weight[2]; mz_maxprob=z_gemplusminus.M(); }
               if (maxprob<weight[3]){ maxprob=weight[3]; mz_maxprob=z_gemminusplus.M(); }

               h_mz_find_peak->Fill(z_gemplus.M(), weight[0]);
               h_mz_find_peak->Fill(z_gemminus.M(), weight[1]);
               h_mz_find_peak->Fill(z_gemplusminus.M(), weight[2]);
               h_mz_find_peak->Fill(z_gemminusplus.M(), weight[3]);
                  
	    } // m
	  }   // m
	}	  // ety
      }		  // etx
    }		  // phi
  } 		  // phi

  // chosing the 'best' Z-mass for diffrent methods

  if( _debug){ cout << "MMC: 6D scan: Picking solutions..." << endl; }
  //               cout << "MMC: mem   "   << &v_weight       << "==" << &v_zm_best << endl;
  //               cout << "MMC: sizes "   << v_weight.size() << "==" << v_zm_best.size() << endl;}

  // takes the value with the highest probability
  //  float mz_maxprob = GetZmass_Max(&v_weight, &v_zm_best);
  this->SetMZ_maxprob(mz_maxprob);

  if( _debug) cout << "MMC: maxprob=" << mz_maxprob << endl;
  
  // takes the mean value weighted by DeltaR
  float mz_mean = -1; if (weightsum>0) mz_mean=mzweightedsum/weightsum; //GetZmass_Weighted( &v_weight, &v_zm_best);
  this->SetMZ_mean(mz_mean);
  cout << weightsum << endl;

  if( _debug) cout << "MMC: mean=" << mz_mean << endl;

  // takes the peak-value from the weighted histogram of all possible Z-masses
  float mz_peak;
  if (h_mz_find_peak->GetEntries()==0){
    mz_peak=-1;
  } else{
    int maxbin=h_mz_find_peak->GetMaximumBin();
    mz_peak = h_mz_find_peak->GetBinCenter(maxbin);		
  }
  delete h_mz_find_peak;
  this->SetMZ_peak(mz_peak);

  if( _debug) cout << "MMC: peak=" << mz_peak << endl;
 
  //  if (_debug) cout << " max: " << mz_maxprob << " weight: " << mz_mean << " peak: " << mz_peak << endl;

  cout << endl << " max: " << mz_maxprob << " weight: " << mz_mean << " peak: " << mz_peak << endl;
    
  if( _debug) cout << __PRETTY_FUNCTION__ << endl;

  if( _debug) cout << "MMC: 6D scan: Done" << endl;

  c_t1=clock()-c_t0;
  m_exectime=c_t1/(1.0*CLOCKS_PER_SEC);

  if( _debug) cout << "Execution time: " << m_exectime << endl;

  return mz_maxprob;
}

float mmc::Scan6dTheta(const TLorentzVector _lep1, const TLorentzVector _lep2, const float _metx, const float _mety, const float _sumet)
{

  clock_t c_t0=clock(),c_t1;
  const float MIN_PT_GeV = 1.;	// use solutions with missing PT>1GeV in ThetaPhi scan

  m_mz_peak=-1;
  m_mz_mean=-1;
  m_mz_maxprob=-1;
  Corr_met_ex=_metx;
  Corr_met_ey=_mety;
  
  TH1F* h_mz_find_peak = new TH1F("h_mz_find_peak", "h_mz_find_peak", 500, 0., 500.);
  TH2F* h_mz_find_MET = new TH2F("h_mz_find_MET", "h_mz_find_MET", 2*MaxCorrMET,-MaxCorrMET,MaxCorrMET, 2*MaxCorrMET,-MaxCorrMET,MaxCorrMET);

  const float sigma = m_noise + m_alpha * sqrt(_sumet);
 // const float mey_sigma = m_alpha * sqrt(_sumet);

    //to speed things up - use floats instead of TLV
	float Evis_el = _lep1.E(), Evis_mu = _lep2.E();
	float lt_el = _lep1.Pt(), lt_mu = _lep2.Pt();
	float lx_el = _lep1.Px(), lx_mu = _lep2.Px();
	float ly_el = _lep1.Py(), ly_mu = _lep2.Py();
	float lz_el = _lep1.Pz(), lz_mu = _lep2.Pz();
	float l_el = _lep1.P(), l_mu = _lep2.P();		
	float l_el_phi = _lep1.Phi(), l_mu_phi = _lep2.Phi();		
	float l_el_theta = _lep1.Theta(), l_mu_theta = _lep2.Theta();		
	float l_el_sintheta = Sin(l_el_theta), l_mu_sintheta = Sin(l_mu_theta);	
	float l_el_costheta = Cos(l_el_theta), l_mu_costheta = Cos(l_mu_theta);	

	float mass1 = _lep1.M(), mass2= _lep2.M();
	float vis_m_pos, vis_m_neg;
	if(mass2>mass1) vis_m_pos = 0.000511, vis_m_neg=0.105658;
	else vis_m_neg = 0.000511, vis_m_pos=0.105658;

	float vis_m_pos2 = vis_m_pos*vis_m_pos, vis_m_neg2=vis_m_neg*vis_m_neg;
	
	float M_tau4 = M_tau*M_tau*M_tau*M_tau, vis_m_pos4 = vis_m_pos2*vis_m_pos2, vis_m_neg4 = vis_m_neg2*vis_m_neg2;
	float Norm_el = (8*M_tau4)/( (M_tau4-vis_m_pos4)*(M_tau4-8*M_tau*M_tau*vis_m_pos2+vis_m_pos4) + 24.*vis_m_pos4*M_tau4*Log(M_tau/vis_m_pos) );
	float Norm_mu = (8*M_tau4)/( (M_tau4-vis_m_neg4)*(M_tau4-8*M_tau*M_tau*vis_m_neg2+vis_m_neg4) + 24.*vis_m_neg4*M_tau4*Log(M_tau/vis_m_neg) );

	float phi_step = 2.*m_phi_lim / m_phi_n;
	float theta_step = 2.*m_theta_lim / m_theta_n;
	float cos_alpha_el, cos_alpha_mu, Res_mass;
	float E_tau_el, E_tau_mu, E_miss_el, E_miss_mu, miss_vis_m_pos, miss_vis_m_neg, P_tau_el, P_tau_mu;

//Loop 1,2 MET loop
for(float del_phi=0.1;del_phi<2.*Pi()+0.1;del_phi+=2.*Pi()/MET_scan_points_Phi){
for(float delZ=0.99;delZ>0;delZ-=0.98/MET_scan_points_Z){
	float MET_counter=0;
	float Ex_mes = _metx - Sqrt(-2*Log(delZ))*sigma*Cos(del_phi);
	float Ey_mes = _mety - Sqrt(-2*Log(delZ))*sigma*Sin(del_phi);

// Loop 3,4 phi scan
	for (float phi_el = l_el_phi - m_phi_lim ; phi_el <= l_el_phi + m_phi_lim ; phi_el += phi_step ) {
	for (float phi_mu = l_mu_phi - m_phi_lim ; phi_mu <= l_mu_phi + m_phi_lim ; phi_mu += phi_step ) {
		float P_el_sinphi = Sin(phi_el), P_mu_sinphi = Sin(phi_mu);
		float P_el_cosphi = Cos(phi_el), P_mu_cosphi = Cos(phi_mu);
		float PT_el = (Ex_mes*P_mu_sinphi - Ey_mes*P_mu_cosphi)/Sin(phi_mu-phi_el);
		float PT_mu = (Ex_mes*P_el_sinphi - Ey_mes*P_el_cosphi)/Sin(phi_el-phi_mu);
		if (PT_el<MIN_PT_GeV || PT_mu < MIN_PT_GeV ) continue;	// PT>0 always
				
// Loop 5,6 theta scan
		float theta_el_lim = Min( (float) Pi(),l_el_theta + m_theta_lim), theta_mu_lim = Min( (float) Pi(),l_mu_theta + m_theta_lim);
		float zero=0.15;	// set maximum eta for missing di-nu system -> eta > 6.6
		for (float theta_el = Max(zero,l_el_theta - m_theta_lim) ; theta_el <= theta_el_lim ; theta_el += theta_step ) {
		for (float theta_mu = Max(zero,l_mu_theta - m_theta_lim) ; theta_mu <= theta_mu_lim ; theta_mu += theta_step ) {

		float P_el = PT_el/Sin(theta_el), P_mu = PT_mu/Sin(theta_mu);
// Calculating the missing vectors
		float P_el_costheta=Cos(theta_el), P_mu_costheta=Cos(theta_mu);
		cos_alpha_el = l_el_sintheta*Sin(theta_el)*Cos(l_el_phi-phi_el)+l_el_costheta*P_el_costheta;
		cos_alpha_mu = l_mu_sintheta*Sin(theta_mu)*Cos(l_mu_phi-phi_mu)+l_mu_costheta*P_mu_costheta;
		P_tau_el = Sqrt(l_el*l_el + P_el*P_el + 2.*P_el*l_el*cos_alpha_el);
		P_tau_mu = Sqrt(l_mu*l_mu + P_mu*P_mu + 2.*P_mu*l_mu*cos_alpha_mu);
		E_tau_el = Sqrt(M_tau*M_tau + P_tau_el*P_tau_el);
		E_tau_mu = Sqrt(M_tau*M_tau + P_tau_mu*P_tau_mu);
		E_miss_el=E_tau_el-Evis_el; E_miss_mu=E_tau_mu-Evis_mu;
		if(E_miss_el<=P_el) continue;
		if(E_miss_mu<=P_mu) continue;	
		miss_vis_m_pos = Sqrt(E_miss_el*E_miss_el-P_el*P_el), miss_vis_m_neg = Sqrt(E_miss_mu*E_miss_mu-P_mu*P_mu);
		if( (miss_vis_m_pos>M_tau-vis_m_pos) || (miss_vis_m_neg>M_tau-vis_m_neg)) continue;
		float pz_el = P_el*P_el_costheta, pz_mu = P_mu*P_mu_costheta;
		
		Res_mass = Sqrt((E_tau_el+E_tau_mu)*(E_tau_el+E_tau_mu) - (lx_el+lx_mu+Ex_mes)*(lx_el+lx_mu+Ex_mes) - 
		(ly_el+ly_mu+Ey_mes)*(ly_el+ly_mu+Ey_mes) - (lz_el+lz_mu+pz_mu+pz_el)*(lz_el+lz_mu+pz_mu+pz_el));

		if (Res_mass<20. || Res_mass>500.) continue;	

// Calculating the Probability
		float beta_el = P_tau_el/E_tau_el, beta_mu = P_tau_mu/E_tau_mu;
		float gamma_el = 1./Sqrt(1-beta_el*beta_el), gamma_mu = 1./Sqrt(1-beta_mu*beta_mu);
		float Estar_el = 0.5*(M_tau*M_tau + vis_m_pos2 - miss_vis_m_pos*miss_vis_m_pos)/M_tau;
		float Estar_mu = 0.5*(M_tau*M_tau + vis_m_neg2 - miss_vis_m_neg*miss_vis_m_neg)/M_tau;
		float Pstar_el = 0.5*Sqrt((M_tau*M_tau - vis_m_pos2 - miss_vis_m_pos*miss_vis_m_pos)*(M_tau*M_tau - vis_m_pos2 - miss_vis_m_pos*miss_vis_m_pos) - 4.*miss_vis_m_pos*miss_vis_m_pos*vis_m_pos2)/M_tau;
		float Pstar_mu = 0.5*Sqrt((M_tau*M_tau - vis_m_neg2 - miss_vis_m_neg*miss_vis_m_neg)*(M_tau*M_tau - vis_m_neg2 - miss_vis_m_neg*miss_vis_m_neg) - 4.*miss_vis_m_neg*miss_vis_m_neg*vis_m_neg2)/M_tau;
		float cos_theta_el = (gamma_el*Estar_el-Evis_el)/(gamma_el*beta_el*Pstar_el);
		float cos_theta_mu = (gamma_mu*Estar_mu-Evis_mu)/(gamma_mu*beta_mu*Pstar_mu);

		// calculate the probability for P(cos_theta,m2nu) for both taus have same polarization (s=1)
		float S_tau_el = 1, S_tau_mu = 1;
		float Prob_el = Pstar_el*(2.*M_tau*(vis_m_pos2+2.*Estar_el*Estar_el)+3.*Estar_el*(M_tau*M_tau+vis_m_pos2)) - 
						Pstar_el*Pstar_el*S_tau_el*cos_theta_el*(3.*vis_m_pos2+M_tau*M_tau+4.*M_tau*Estar_el);
						Prob_el*=Norm_el*miss_vis_m_pos;
		float Prob_mu = Pstar_mu*(2.*M_tau*(vis_m_neg2+2.*Estar_mu*Estar_mu)+3.*Estar_mu*(M_tau*M_tau+vis_m_neg2)) - 
						Pstar_mu*Pstar_mu*S_tau_mu*cos_theta_mu*(3.*vis_m_neg2+M_tau*M_tau+4.*M_tau*Estar_mu);
						Prob_mu*=Norm_mu*miss_vis_m_neg;
		float Probability = Prob_el*Prob_mu;

		double dphi_el = l_el_phi - phi_el ,dphi_mu = l_mu_phi- phi_mu;

		// solving the Jacobian
		float dLogPTmis_el_dphi_el = -1. / Tan(phi_mu - phi_el);
		float dLogPTmis_el_dphi_mu = -dLogPTmis_el_dphi_el - (Ex_mes*Cos(phi_mu)+ Ey_mes*Sin(phi_mu))/(PT_el*Sin(phi_mu-phi_el));
		float dLogPTmis_mu_dphi_mu = -dLogPTmis_el_dphi_el;	
		float dLogPTmis_mu_dphi_el = -dLogPTmis_mu_dphi_mu - (Ex_mes*Cos(phi_el)+ Ey_mes*Sin(phi_el))/(PT_mu*Sin(phi_el-phi_mu));
							
		float dPtau_el_dphi_el = 2.*dLogPTmis_el_dphi_el*P_el*(P_el+l_el*cos_alpha_el)-2.*PT_el*lt_el*Sin(dphi_el);
		float dPtau_el_dphi_mu = 2.*dLogPTmis_el_dphi_mu*P_el*(P_el+l_el*cos_alpha_el);
		float dPtau_el_dtheta_el = 2.*P_el*P_el/PT_el*(pz_el+lz_el);
		float dPtau_mu_dphi_el = 2.*dLogPTmis_mu_dphi_el*P_mu*(P_mu+l_mu*cos_alpha_mu);
		float dPtau_mu_dphi_mu = 2.*dLogPTmis_mu_dphi_mu*P_mu*(P_mu+l_mu*cos_alpha_mu)-2.*PT_mu*lt_mu*Sin(dphi_mu);
		float dPtau_mu_dtheta_mu	= 2.*P_mu*P_mu/PT_mu*(pz_mu+lz_mu);				
					
		float dbeta_el_dphi_el = dPtau_el_dphi_el / (2.*beta_el*gamma_el*gamma_el*E_tau_el*E_tau_el);
		float dbeta_el_dphi_mu = dPtau_el_dphi_mu / (2.*beta_el*gamma_el*gamma_el*E_tau_el*E_tau_el);
		float dbeta_el_dtheta_el = dPtau_el_dtheta_el / (2.*beta_el*gamma_el*gamma_el*E_tau_el*E_tau_el);
		float dbeta_mu_dphi_el = dPtau_mu_dphi_el / (2.*beta_mu*gamma_mu*gamma_mu*E_tau_mu*E_tau_mu);
		float dbeta_mu_dphi_mu = dPtau_mu_dphi_mu / (2.*beta_mu*gamma_mu*gamma_mu*E_tau_mu*E_tau_mu);
		float dbeta_mu_dtheta_mu = dPtau_mu_dtheta_mu / (2.*beta_mu*gamma_mu*gamma_mu*E_tau_mu*E_tau_mu);

//		float dm_el_dphi_el = (0.5*E_miss_el/E_tau_el*dPtau_el_dphi_el-P_el*P_el*dLogPTmis_el_dphi_el)/miss_vis_m_pos;
//		float dm_el_dphi_mu = (0.5*E_miss_el/E_tau_el*dPtau_el_dphi_mu-P_el*P_el*dLogPTmis_el_dphi_mu)/miss_vis_m_pos;
//		float dm_el_dtheta_el = (0.5*E_miss_el/E_tau_el*dPtau_el_dtheta_el - P_el*P_el*pz_el/PT_el)/miss_vis_m_pos;
//		float dm_mu_dphi_el = (0.5*E_miss_mu/E_tau_mu*dPtau_mu_dphi_el-P_mu*P_mu*dLogPTmis_mu_dphi_el)/miss_vis_m_neg;
//		float dm_mu_dphi_mu = (0.5*E_miss_mu/E_tau_mu*dPtau_mu_dphi_mu-P_mu*P_mu*dLogPTmis_mu_dphi_mu)/miss_vis_m_neg;
//		float dm_mu_dtheta_mu = (0.5*E_miss_mu/E_tau_mu*dPtau_mu_dtheta_mu-P_mu*P_mu*pz_mu/PT_mu)/miss_vis_m_neg;

		float dm_el_dphi_el = (-0.5*Evis_el/E_tau_el*dPtau_el_dphi_el+P_el*l_el*cos_alpha_el*dLogPTmis_el_dphi_el-PT_el*lt_el*Sin(dphi_el))/miss_vis_m_pos;
		float dm_el_dphi_mu = (-0.5*Evis_el/E_tau_el*dPtau_el_dphi_mu-P_el*l_el*cos_alpha_el*dLogPTmis_el_dphi_mu)/miss_vis_m_pos;
		float dm_el_dtheta_el = (-0.5*Evis_el/E_tau_el*dPtau_el_dtheta_el - P_el*P_el*lz_el/PT_el)/miss_vis_m_pos;
		float dm_mu_dphi_el = (-0.5*Evis_mu/E_tau_mu*dPtau_mu_dphi_el+P_mu*l_mu*cos_alpha_mu*dLogPTmis_mu_dphi_el)/miss_vis_m_neg;
		float dm_mu_dphi_mu = (-0.5*Evis_mu/E_tau_mu*dPtau_mu_dphi_mu+P_mu*l_mu*cos_alpha_mu*dLogPTmis_mu_dphi_mu-PT_mu*lt_mu*Sin(dphi_mu))/miss_vis_m_neg;
		float dm_mu_dtheta_mu = (-0.5*Evis_mu/E_tau_mu*dPtau_mu_dtheta_mu-P_mu*P_mu*lz_mu/PT_mu)/miss_vis_m_neg;
		
		float dcostheta_el_dphi_el = ((gamma_el*Evis_el-Estar_el)*dbeta_el_dphi_el - beta_el*miss_vis_m_pos/(M_tau*gamma_el)*(gamma_el*vis_m_pos2-Estar_el*Evis_el)/(Pstar_el*Pstar_el)*dm_el_dphi_el)/(beta_el*beta_el*Pstar_el);
		float dcostheta_el_dphi_mu = ((gamma_el*Evis_el-Estar_el)*dbeta_el_dphi_mu - beta_el*miss_vis_m_pos/(M_tau*gamma_el)*(gamma_el*vis_m_pos2-Estar_el*Evis_el)/(Pstar_el*Pstar_el)*dm_el_dphi_mu)/(beta_el*beta_el*Pstar_el);
		float dcostheta_el_dtheta_el = ((gamma_el*Evis_el-Estar_el)*dbeta_el_dtheta_el - beta_el*miss_vis_m_pos/(M_tau*gamma_el)*(gamma_el*vis_m_pos2-Estar_el*Evis_el)/(Pstar_el*Pstar_el)*dm_el_dtheta_el)/(beta_el*beta_el*Pstar_el);
		float dcostheta_mu_dphi_el = ((gamma_mu*Evis_mu-Estar_mu)*dbeta_mu_dphi_el - beta_mu*miss_vis_m_neg/(M_tau*gamma_mu)*(gamma_mu*vis_m_neg2-Estar_mu*Evis_mu)/(Pstar_mu*Pstar_mu)*dm_mu_dphi_el)/(beta_mu*beta_mu*Pstar_mu);
		float dcostheta_mu_dphi_mu = ((gamma_mu*Evis_mu-Estar_mu)*dbeta_mu_dphi_mu - beta_mu*miss_vis_m_neg/(M_tau*gamma_mu)*(gamma_mu*vis_m_neg2-Estar_mu*Evis_mu)/(Pstar_mu*Pstar_mu)*dm_mu_dphi_mu)/(beta_mu*beta_mu*Pstar_mu);
		float dcostheta_mu_dtheta_mu = ((gamma_mu*Evis_mu-Estar_mu)*dbeta_mu_dtheta_mu - beta_mu*miss_vis_m_neg/(M_tau*gamma_mu)*(gamma_mu*vis_m_neg2-Estar_mu*Evis_mu)/(Pstar_mu*Pstar_mu)*dm_mu_dtheta_mu)/(beta_mu*beta_mu*Pstar_mu);
							
		float Jacobian =	dcostheta_el_dphi_el * dcostheta_mu_dphi_mu * dm_el_dtheta_el * dm_mu_dtheta_mu - 
							dcostheta_el_dphi_el * dcostheta_mu_dtheta_mu * dm_el_dtheta_el * dm_mu_dphi_mu -
							dcostheta_el_dphi_mu * dcostheta_mu_dphi_el * dm_el_dtheta_el * dm_mu_dtheta_mu +
							dcostheta_el_dphi_mu * dcostheta_mu_dtheta_mu * dm_el_dtheta_el * dm_mu_dphi_el +
							dcostheta_el_dtheta_el * dcostheta_mu_dphi_el * dm_el_dphi_mu * dm_mu_dtheta_mu -
							dcostheta_el_dtheta_el * dcostheta_mu_dphi_mu * dm_el_dphi_el * dm_mu_dtheta_mu +
							dcostheta_el_dtheta_el * dcostheta_mu_dtheta_mu * dm_el_dphi_el * dm_mu_dphi_mu -
							dcostheta_el_dtheta_el * dcostheta_mu_dtheta_mu * dm_el_dphi_mu * dm_mu_dphi_el;
						
		Probability *= Abs(Jacobian);
		MET_counter++;
	
// Fill the mass histogramm
		h_mz_find_peak->Fill(Res_mass,Probability);
		
	    } // theta mu
		} // theta el
	} // phi mu
    } // phi el
	if(MET_counter)
		h_mz_find_MET->Fill( _metx - Ex_mes, _mety - Ey_mes , MET_counter);

} // MET_Z
} // MET_Phi

  if(h_mz_find_peak->GetEntries()){
	  this->SetMZ_mean(h_mz_find_peak->GetMean());
	  this->SetMZ_peak(h_mz_find_peak->GetBinCenter(h_mz_find_peak->GetMaximumBin()));
	  Corr_met_ex=_metx-h_mz_find_MET->GetMean(1);
	  Corr_met_ey=_mety-h_mz_find_MET->GetMean(2);
  }
  this->SetMZ_maxprob(-1.);
 
  delete h_mz_find_peak;
  delete h_mz_find_MET;

  c_t1=clock()-c_t0;
  m_exectime=c_t1/(1.0*CLOCKS_PER_SEC);
   
  return 1.;
}

float mmc::Scan4dTheta(const TLorentzVector _lep1, const TLorentzVector _lep2, const float _metx, const float _mety)
{

  clock_t c_t0=clock(),c_t1;
  const float MIN_PT_GeV = 1.;	// use solutions with missing PT>1GeV in ThetaPhi scan

  m_mz_peak=-1;
  m_mz_mean=-1;
  m_mz_maxprob=-1;

  
  TH1F* h_mz_find_peak = new TH1F("h_mz_find_peak", "h_mz_find_peak", 500, 0., 500.);

    //to speed things up - use floats instead of TLV
	float Evis_el = _lep1.E(), Evis_mu = _lep2.E();
	float lt_el = _lep1.Pt(), lt_mu = _lep2.Pt();
	float lx_el = _lep1.Px(), lx_mu = _lep2.Px();
	float ly_el = _lep1.Py(), ly_mu = _lep2.Py();
	float lz_el = _lep1.Pz(), lz_mu = _lep2.Pz();
	float l_el = _lep1.P(), l_mu = _lep2.P();		
	float l_el_phi = _lep1.Phi(), l_mu_phi = _lep2.Phi();		
	float l_el_theta = _lep1.Theta(), l_mu_theta = _lep2.Theta();		
	float l_el_sintheta = Sin(l_el_theta), l_mu_sintheta = Sin(l_mu_theta);	
	float l_el_costheta = Cos(l_el_theta), l_mu_costheta = Cos(l_mu_theta);	

	float mass1 = _lep1.M(), mass2= _lep2.M();
	float vis_m_pos, vis_m_neg;
	if(mass2>mass1) vis_m_pos = 0.000511, vis_m_neg=0.105658;
	else vis_m_neg = 0.000511, vis_m_pos=0.105658;

	float vis_m_pos2 = vis_m_pos*vis_m_pos, vis_m_neg2=vis_m_neg*vis_m_neg;
	float M_tau4 = M_tau*M_tau*M_tau*M_tau, vis_m_pos4 = vis_m_pos2*vis_m_pos2, vis_m_neg4 = vis_m_neg2*vis_m_neg2;
	float Norm_el = (8*M_tau4)/( (M_tau4-vis_m_pos4)*(M_tau4-8*M_tau*M_tau*vis_m_pos2+vis_m_pos4) + 24.*vis_m_pos4*M_tau4*Log(M_tau/vis_m_pos) );
	float Norm_mu = (8*M_tau4)/( (M_tau4-vis_m_neg4)*(M_tau4-8*M_tau*M_tau*vis_m_neg2+vis_m_neg4) + 24.*vis_m_neg4*M_tau4*Log(M_tau/vis_m_neg) );

	float phi_step = 2.*m_phi_lim / m_phi_n;
	float theta_step = 2.*m_theta_lim / m_theta_n;
	float cos_alpha_el, cos_alpha_mu, Res_mass;
	float E_tau_el, E_tau_mu, E_miss_el, E_miss_mu, miss_vis_m_pos, miss_vis_m_neg, P_tau_el, P_tau_mu;
	float Ex_mes = _metx;
	float Ey_mes = _mety;

// Loop 1,2 phi scan
	for (float phi_el = l_el_phi - m_phi_lim ; phi_el <= l_el_phi + m_phi_lim ; phi_el += phi_step ) {
	for (float phi_mu = l_mu_phi - m_phi_lim ; phi_mu <= l_mu_phi + m_phi_lim ; phi_mu += phi_step ) {
		float P_el_sinphi = Sin(phi_el), P_mu_sinphi = Sin(phi_mu);
		float P_el_cosphi = Cos(phi_el), P_mu_cosphi = Cos(phi_mu);
		float PT_el = (Ex_mes*P_mu_sinphi - Ey_mes*P_mu_cosphi)/Sin(phi_mu-phi_el);
		float PT_mu = (Ex_mes*P_el_sinphi - Ey_mes*P_el_cosphi)/Sin(phi_el-phi_mu);
		if (PT_el<MIN_PT_GeV || PT_mu < MIN_PT_GeV ) continue;	// PT>0 always
				
// Loop 3,4 theta scan
		float theta_el_lim = Min( (float) Pi(),l_el_theta + m_theta_lim), theta_mu_lim = Min( (float) Pi(),l_mu_theta + m_theta_lim);
		float zero=0.15;	// set maximum eta for missing di-nu system -> eta > 6.6
		for (float theta_el = Max(zero,l_el_theta - m_theta_lim) ; theta_el <= theta_el_lim ; theta_el += theta_step ) {
		for (float theta_mu = Max(zero,l_mu_theta - m_theta_lim) ; theta_mu <= theta_mu_lim ; theta_mu += theta_step ) {
		
		float P_el = PT_el/Sin(theta_el), P_mu = PT_mu/Sin(theta_mu);
// Calculating the missing vectors
		float P_el_costheta=Cos(theta_el), P_mu_costheta=Cos(theta_mu);
		cos_alpha_el = l_el_sintheta*Sin(theta_el)*Cos(l_el_phi-phi_el)+l_el_costheta*P_el_costheta;
		cos_alpha_mu = l_mu_sintheta*Sin(theta_mu)*Cos(l_mu_phi-phi_mu)+l_mu_costheta*P_mu_costheta;
		P_tau_el = Sqrt(l_el*l_el + P_el*P_el + 2.*P_el*l_el*cos_alpha_el);
		P_tau_mu = Sqrt(l_mu*l_mu + P_mu*P_mu + 2.*P_mu*l_mu*cos_alpha_mu);
		E_tau_el = Sqrt(M_tau*M_tau + P_tau_el*P_tau_el);
		E_tau_mu = Sqrt(M_tau*M_tau + P_tau_mu*P_tau_mu);
		E_miss_el=E_tau_el-Evis_el; E_miss_mu=E_tau_mu-Evis_mu;
		if(E_miss_el<=P_el) continue;
		if(E_miss_mu<=P_mu) continue;	
		miss_vis_m_pos = Sqrt(E_miss_el*E_miss_el-P_el*P_el), miss_vis_m_neg = Sqrt(E_miss_mu*E_miss_mu-P_mu*P_mu);
		if( (miss_vis_m_pos>M_tau-vis_m_pos) || (miss_vis_m_neg>M_tau-vis_m_neg)) continue;
		float pz_el = P_el*P_el_costheta, pz_mu = P_mu*P_mu_costheta;
		
		Res_mass = Sqrt((E_tau_el+E_tau_mu)*(E_tau_el+E_tau_mu) - (lx_el+lx_mu+Ex_mes)*(lx_el+lx_mu+Ex_mes) - 
		(ly_el+ly_mu+Ey_mes)*(ly_el+ly_mu+Ey_mes) - (lz_el+lz_mu+pz_mu+pz_el)*(lz_el+lz_mu+pz_mu+pz_el));

		if (Res_mass<20. || Res_mass>500.) continue;	

// Calculating the Probability
		float beta_el = P_tau_el/E_tau_el, beta_mu = P_tau_mu/E_tau_mu;
		float gamma_el = 1./Sqrt(1-beta_el*beta_el), gamma_mu = 1./Sqrt(1-beta_mu*beta_mu);
		float Estar_el = 0.5*(M_tau*M_tau + vis_m_pos*vis_m_pos - miss_vis_m_pos*miss_vis_m_pos)/M_tau;
		float Estar_mu = 0.5*(M_tau*M_tau + vis_m_neg*vis_m_neg - miss_vis_m_neg*miss_vis_m_neg)/M_tau;
		float Pstar_el = 0.5*Sqrt((M_tau*M_tau - vis_m_pos*vis_m_pos - miss_vis_m_pos*miss_vis_m_pos)*(M_tau*M_tau - vis_m_pos*vis_m_pos - miss_vis_m_pos*miss_vis_m_pos) - 4.*miss_vis_m_pos*miss_vis_m_pos*vis_m_pos*vis_m_pos)/M_tau;
		float Pstar_mu = 0.5*Sqrt((M_tau*M_tau - vis_m_neg*vis_m_neg - miss_vis_m_neg*miss_vis_m_neg)*(M_tau*M_tau - vis_m_neg*vis_m_neg - miss_vis_m_neg*miss_vis_m_neg) - 4.*miss_vis_m_neg*miss_vis_m_neg*vis_m_neg*vis_m_neg)/M_tau;
		float cos_theta_el = (gamma_el*Estar_el-Evis_el)/(gamma_el*beta_el*Pstar_el);
		float cos_theta_mu = (gamma_mu*Estar_mu-Evis_mu)/(gamma_mu*beta_mu*Pstar_mu);

		// calculate the probability for P(cos_theta,m2nu) for both taus have same polarization (s=1)
		float S_tau_el = 1, S_tau_mu = -1;
		float Prob_el = Pstar_el*(2.*M_tau*(vis_m_pos2+2.*Estar_el*Estar_el)+3.*Estar_el*(M_tau*M_tau+vis_m_pos2)) - 
						Pstar_el*Pstar_el*S_tau_el*cos_theta_el*(3.*vis_m_pos2+M_tau*M_tau+4.*M_tau*Estar_el);
						Prob_el*=Norm_el*miss_vis_m_pos;
		float Prob_mu = Pstar_mu*(2.*M_tau*(vis_m_neg2+2.*Estar_mu*Estar_mu)+3.*Estar_mu*(M_tau*M_tau+vis_m_neg2)) - 
						Pstar_mu*Pstar_mu*S_tau_mu*cos_theta_mu*(3.*vis_m_neg2+M_tau*M_tau+4.*M_tau*Estar_mu);
						Prob_mu*=Norm_mu*miss_vis_m_neg;
						
		float Probability = Prob_el*Prob_mu;
		
		double dphi_el = l_el_phi - phi_el ,dphi_mu = l_mu_phi- phi_mu;

		// solving the Jacobian
		float dLogPTmis_el_dphi_el = -1. / Tan(phi_mu - phi_el);
		float dLogPTmis_el_dphi_mu = -dLogPTmis_el_dphi_el - (Ex_mes*Cos(phi_mu)+ Ey_mes*Sin(phi_mu))/(PT_el*Sin(phi_mu-phi_el));
		float dLogPTmis_mu_dphi_mu = -dLogPTmis_el_dphi_el;	
		float dLogPTmis_mu_dphi_el = -dLogPTmis_mu_dphi_mu - (Ex_mes*Cos(phi_el)+ Ey_mes*Sin(phi_el))/(PT_mu*Sin(phi_el-phi_mu));
							
		float dPtau_el_dphi_el = 2.*dLogPTmis_el_dphi_el*P_el*(P_el+l_el*cos_alpha_el)-2.*PT_el*lt_el*Sin(dphi_el);
		float dPtau_el_dphi_mu = 2.*dLogPTmis_el_dphi_mu*P_el*(P_el+l_el*cos_alpha_el);
		float dPtau_el_dtheta_el = 2.*P_el*P_el/PT_el*(pz_el+lz_el);
		float dPtau_mu_dphi_el = 2.*dLogPTmis_mu_dphi_el*P_mu*(P_mu+l_mu*cos_alpha_mu);
		float dPtau_mu_dphi_mu = 2.*dLogPTmis_mu_dphi_mu*P_mu*(P_mu+l_mu*cos_alpha_mu)-2.*PT_mu*lt_mu*Sin(dphi_mu);
		float dPtau_mu_dtheta_mu	= 2.*P_mu*P_mu/PT_mu*(pz_mu+lz_mu);				
					
		float dbeta_el_dphi_el = dPtau_el_dphi_el / (2.*beta_el*gamma_el*gamma_el*E_tau_el*E_tau_el);
		float dbeta_el_dphi_mu = dPtau_el_dphi_mu / (2.*beta_el*gamma_el*gamma_el*E_tau_el*E_tau_el);
		float dbeta_el_dtheta_el = dPtau_el_dtheta_el / (2.*beta_el*gamma_el*gamma_el*E_tau_el*E_tau_el);
		float dbeta_mu_dphi_el = dPtau_mu_dphi_el / (2.*beta_mu*gamma_mu*gamma_mu*E_tau_mu*E_tau_mu);
		float dbeta_mu_dphi_mu = dPtau_mu_dphi_mu / (2.*beta_mu*gamma_mu*gamma_mu*E_tau_mu*E_tau_mu);
		float dbeta_mu_dtheta_mu = dPtau_mu_dtheta_mu / (2.*beta_mu*gamma_mu*gamma_mu*E_tau_mu*E_tau_mu);

//		float dm_el_dphi_el = (0.5*E_miss_el/E_tau_el*dPtau_el_dphi_el-P_el*P_el*dLogPTmis_el_dphi_el)/miss_vis_m_pos;
//		float dm_el_dphi_mu = (0.5*E_miss_el/E_tau_el*dPtau_el_dphi_mu-P_el*P_el*dLogPTmis_el_dphi_mu)/miss_vis_m_pos;
//		float dm_el_dtheta_el = (0.5*E_miss_el/E_tau_el*dPtau_el_dtheta_el - P_el*P_el*pz_el/PT_el)/miss_vis_m_pos;
//		float dm_mu_dphi_el = (0.5*E_miss_mu/E_tau_mu*dPtau_mu_dphi_el-P_mu*P_mu*dLogPTmis_mu_dphi_el)/miss_vis_m_neg;
//		float dm_mu_dphi_mu = (0.5*E_miss_mu/E_tau_mu*dPtau_mu_dphi_mu-P_mu*P_mu*dLogPTmis_mu_dphi_mu)/miss_vis_m_neg;
//		float dm_mu_dtheta_mu = (0.5*E_miss_mu/E_tau_mu*dPtau_mu_dtheta_mu-P_mu*P_mu*pz_mu/PT_mu)/miss_vis_m_neg;

		float dm_el_dphi_el = (-0.5*Evis_el/E_tau_el*dPtau_el_dphi_el+P_el*l_el*cos_alpha_el*dLogPTmis_el_dphi_el-PT_el*lt_el*Sin(dphi_el))/miss_vis_m_pos;
		float dm_el_dphi_mu = (-0.5*Evis_el/E_tau_el*dPtau_el_dphi_mu-P_el*l_el*cos_alpha_el*dLogPTmis_el_dphi_mu)/miss_vis_m_pos;
		float dm_el_dtheta_el = (-0.5*Evis_el/E_tau_el*dPtau_el_dtheta_el - P_el*P_el*lz_el/PT_el)/miss_vis_m_pos;
		float dm_mu_dphi_el = (-0.5*Evis_mu/E_tau_mu*dPtau_mu_dphi_el+P_mu*l_mu*cos_alpha_mu*dLogPTmis_mu_dphi_el)/miss_vis_m_neg;
		float dm_mu_dphi_mu = (-0.5*Evis_mu/E_tau_mu*dPtau_mu_dphi_mu+P_mu*l_mu*cos_alpha_mu*dLogPTmis_mu_dphi_mu-PT_mu*lt_mu*Sin(dphi_mu))/miss_vis_m_neg;
		float dm_mu_dtheta_mu = (-0.5*Evis_mu/E_tau_mu*dPtau_mu_dtheta_mu-P_mu*P_mu*lz_mu/PT_mu)/miss_vis_m_neg;
		
		float dcostheta_el_dphi_el = ((gamma_el*Evis_el-Estar_el)*dbeta_el_dphi_el - beta_el*miss_vis_m_pos/(M_tau*gamma_el)*(gamma_el*vis_m_pos2-Estar_el*Evis_el)/(Pstar_el*Pstar_el)*dm_el_dphi_el)/(beta_el*beta_el*Pstar_el);
		float dcostheta_el_dphi_mu = ((gamma_el*Evis_el-Estar_el)*dbeta_el_dphi_mu - beta_el*miss_vis_m_pos/(M_tau*gamma_el)*(gamma_el*vis_m_pos2-Estar_el*Evis_el)/(Pstar_el*Pstar_el)*dm_el_dphi_mu)/(beta_el*beta_el*Pstar_el);
		float dcostheta_el_dtheta_el = ((gamma_el*Evis_el-Estar_el)*dbeta_el_dtheta_el - beta_el*miss_vis_m_pos/(M_tau*gamma_el)*(gamma_el*vis_m_pos2-Estar_el*Evis_el)/(Pstar_el*Pstar_el)*dm_el_dtheta_el)/(beta_el*beta_el*Pstar_el);
		float dcostheta_mu_dphi_el = ((gamma_mu*Evis_mu-Estar_mu)*dbeta_mu_dphi_el - beta_mu*miss_vis_m_neg/(M_tau*gamma_mu)*(gamma_mu*vis_m_neg2-Estar_mu*Evis_mu)/(Pstar_mu*Pstar_mu)*dm_mu_dphi_el)/(beta_mu*beta_mu*Pstar_mu);
		float dcostheta_mu_dphi_mu = ((gamma_mu*Evis_mu-Estar_mu)*dbeta_mu_dphi_mu - beta_mu*miss_vis_m_neg/(M_tau*gamma_mu)*(gamma_mu*vis_m_neg2-Estar_mu*Evis_mu)/(Pstar_mu*Pstar_mu)*dm_mu_dphi_mu)/(beta_mu*beta_mu*Pstar_mu);
		float dcostheta_mu_dtheta_mu = ((gamma_mu*Evis_mu-Estar_mu)*dbeta_mu_dtheta_mu - beta_mu*miss_vis_m_neg/(M_tau*gamma_mu)*(gamma_mu*vis_m_neg2-Estar_mu*Evis_mu)/(Pstar_mu*Pstar_mu)*dm_mu_dtheta_mu)/(beta_mu*beta_mu*Pstar_mu);
							
		float Jacobian =	dcostheta_el_dphi_el * dcostheta_mu_dphi_mu * dm_el_dtheta_el * dm_mu_dtheta_mu - 
							dcostheta_el_dphi_el * dcostheta_mu_dtheta_mu * dm_el_dtheta_el * dm_mu_dphi_mu -
							dcostheta_el_dphi_mu * dcostheta_mu_dphi_el * dm_el_dtheta_el * dm_mu_dtheta_mu +
							dcostheta_el_dphi_mu * dcostheta_mu_dtheta_mu * dm_el_dtheta_el * dm_mu_dphi_el +
							dcostheta_el_dtheta_el * dcostheta_mu_dphi_el * dm_el_dphi_mu * dm_mu_dtheta_mu -
							dcostheta_el_dtheta_el * dcostheta_mu_dphi_mu * dm_el_dphi_el * dm_mu_dtheta_mu +
							dcostheta_el_dtheta_el * dcostheta_mu_dtheta_mu * dm_el_dphi_el * dm_mu_dphi_mu -
							dcostheta_el_dtheta_el * dcostheta_mu_dtheta_mu * dm_el_dphi_mu * dm_mu_dphi_el;
						
		Probability *= Abs(Jacobian);
	
// Fill the mass histogramm
		h_mz_find_peak->Fill(Res_mass,Probability);

	    } // theta mu
		} // theta el
	} // phi mu
    } // phi el

  if(h_mz_find_peak->GetEntries()){
	  this->SetMZ_mean(h_mz_find_peak->GetMean());
	  this->SetMZ_peak(h_mz_find_peak->GetBinCenter(h_mz_find_peak->GetMaximumBin()));
  }
  this->SetMZ_maxprob(-1.);
 
  delete h_mz_find_peak;
  c_t1=clock()-c_t0;
  m_exectime=c_t1/(1.0*CLOCKS_PER_SEC);
 
  return 1.;
}


void mmc::SetAngleWeight(int choice) //default: 2
{
  if (choice==0) this->GetDeltaThetaDistribution();
  else if (choice==1) this->GetDeltaRDistribution();
  else if (choice==2) cout << "Will use analytical weights" << endl;
  else if (choice==3) cout << "Will use analytical weights + Jacobian" << endl;
  else{ cout << "Not a valid weight choice! Allowed are 0, 1 or 2" << endl; }
  m_weight=choice;
}

void mmc::GetDeltaThetaDistribution()
{
  TFile* f_weight = new TFile("weight_functions/dTheta_gaus_taup_higgs.root", "READ");
  const TString fit_bin[nDTHETAWEIGHT]={"10","30","40","50","60","70","80","90","100","110","120","130","140","150","160","170","180","190"};

  for(int i=0; i<nDTHETAWEIGHT; i++)
    f_dTheta_weight[i] = ReadFunc(f_weight, "fit_"+fit_bin[i], "mu");
}

void mmc::GetDeltaRDistribution()
{
  TFile* f_weight = new TFile( "weight_functions/dRCrystalBall_taup.root", "READ");
  const TString fit_bin[nDRWEIGHT]={"10","30","35","40","45","50","55","60","65","70","75","80","85","90","95","100"};

  for(int i=0; i<nDRWEIGHT; i++)
    f_dR_weight[i] = ReadFunc(f_weight, "fit_"+fit_bin[i], "mu");
}

float mmc::GetDeltaRValue(float dR1, float pt1){
  float dRvalue1 = 0;

  if( pt1 > 10 && pt1 < 30) dRvalue1 = f_dR_weight[0]->Eval(dR1);

  int j=1;
  for( int i=30; i<=99; i=i+5){
    if( pt1 > i && pt1 < i+5) dRvalue1 = f_dR_weight[j]->Eval(dR1);
    j++;
  }
  if( pt1 > 100) dRvalue1 = f_dR_weight[nDRWEIGHT-1]->Eval(dR1);

  return dRvalue1;
}

float mmc::GetDeltaThetaValue(float dTheta1, float pt1){
  float dThetavalue1 = 0;

  if( pt1 > 5 && pt1 < 30) dThetavalue1 = f_dTheta_weight[0]->Eval(dTheta1);

  int j=1;
  for( int i=30; i<=199; i=i+10){
    if( pt1 > i && pt1 < i+10) dThetavalue1 = f_dTheta_weight[j]->Eval(dTheta1);
    j++;
  }
  if( pt1 > 200) dThetavalue1 = f_dTheta_weight[nDTHETAWEIGHT-1]->Eval(dTheta1);

  return dThetavalue1;
}

//angle between mmiss and tau
float mmc::GetAnaWeight_TauMiss(const TLorentzVector tau, const TLorentzVector miss, float &ntheta3d){

  float theta3d=fabs(tau.Angle(miss.Vect()));
  float tau_E=tau.E();
  float tau_p=tau.P();
  //  float theta3d=rtheta3d/(tau_p/100);
  ntheta3d=theta3d/(tau_p/100);

  if ( (tau_E-tau_p)<=0 ) return 0;
  float frac=(tau_E+tau_p)/(tau_E-tau_p);
  float denom=pow( 1+cos(ntheta3d)+frac*(1-cos(ntheta3d)),2);
  if (denom<=0) return 0;
  float anaWeight=2*frac*sin(ntheta3d) / denom * (100/tau_p);

  //  cout << tau_p << " " << frac << " " << anaWeight << " " << tau_E << " " << tau_p << " " << theta << endl;
  return anaWeight;
}


void mmc::GetMDistribution()
{
  TFile* f_weight = new TFile( "weight_functions/mpol6_taup.root", "READ");
  f_m_weight = ReadFunc(f_weight, "mfit", "mu");

  return;
}

float mmc::GetMValue(float m1)
{
//  float mvalue1 = f_m_weight->Eval(m1);
	if (m1<1.55 && m1 > 0.05) return 2.11*m1+0.3*m1*m1-2.45*m1*m1*m1+0.91*m1*m1*m1*m1;
	else return 0.;
}


float mmc::GetMETValue(float met, float sigma)
{
  float weight = (exp( -0.5*met*met/(sigma*sigma) )) / ( sqrt(2*TMath::Pi())*sigma );
  return weight;
}

TF1* mmc::ReadFunc(TFile* file, TString histName, TString taumu)
{
  TF1* h_tmp = (TF1*)file->Get(histName);
  TF1* h_histName = (TF1*) h_tmp -> Clone( histName+"_"+taumu);
  return h_histName;
}

/*
float mmc::GetZmass_Max( vector<float> *v_weight, vector<float> *v_zm)
{
  if( _debug){ cout << "MMC: 6D scan: Calculating GetZmass_Max... " << endl;
               cout << "MMC: mem   "   << v_weight << "==" << v_zm << endl;
               cout << "MMC: sizes "   << v_weight->size() << "==" << v_zm->size() << endl;}

  if( v_weight->size() == 0 || v_zm->size() == 0) return 0.;
  if( v_weight->size() != v_zm->size() ){ cout << "ERROR: Something has gone wrong for MaxProb" << endl; return 0.;}
  float max = 0.;
  float max_zm = 0.;
  for( size_t i=0; i<v_weight->size(); i++){
    if( v_weight->at(i) > max){
      max = v_weight->at(i);
      max_zm = v_zm->at(i);
    }
  }

  return max_zm;
}
*/

/*
float mmc::GetZmass_Weighted( vector<float> *v_weight, vector<float> *v_zm)
{
  if( v_weight->size() == 0 || v_zm->size()==0) return 0.;
  if( v_weight->size() != v_zm->size() ){ cout << "ERROR: Something has gone wrong for WeightedMean" << endl; return 0.;}
  float mz_weighted = 0;
  float numerator = 0;
  float denominator = 0;
  for( size_t i=0; i<v_weight->size(); i++){
    numerator += v_weight->at(i)*v_zm->at(i);
    denominator += v_weight->at(i);
  }
  if( denominator != 0) mz_weighted = numerator / denominator;
  else mz_weighted = 0;

  return mz_weighted;
}
*/

float mmc::GetCollMass(const TLorentzVector _lep1, const TLorentzVector _lep2, const float _metx, const float _mety)
{
  float m_mz_coll=0;

  float x1 = ((_lep1.Px()*_lep2.Py())-(_lep1.Py()*_lep2.Px())) / ((_lep1.Px()*_lep2.Py())-(_lep1.Py()*_lep2.Px())+(_lep2.Py()*_metx)-(_lep2.Px()*_mety));
  float x2 = ((_lep1.Px()*_lep2.Py())-(_lep1.Py()*_lep2.Px())) / ((_lep1.Px()*_lep2.Py())-(_lep1.Py()*_lep2.Px())+(_lep1.Px()*_mety)-(_lep1.Py()*_metx));

  if( (x1*x2) > 0. )
    m_mz_coll = ((_lep1+_lep2).M())/(sqrt(x1*x2));
  else m_mz_coll = 0;
//  if(x1*x2 < 0.)
//    m_mz_coll = -(_lep1+_lep2).M()/sqrt(fabs(x1*x2));

  this->SetMZ_coll(m_mz_coll);
  return m_mz_coll;
}

float mmc::GetJacobian(const TLorentzVector vis_pos, const TLorentzVector vis_neg, const TLorentzVector miss_pos, const TLorentzVector miss_neg, const float _Ex_miss, const float _Ey_miss)
{
// input variables: vis_pos, vis_neg, miss_pos, miss_neg, _EX_miss, EY_miss
// output is the Jacobian which is defines the probability: P(phi) = J * P(cos_theta)

// read the variables:
float CTheta_mis_pos = Cos(miss_pos.Theta()), STheta_mis_pos = Sin(miss_pos.Theta());
float CTheta_mis_neg = Cos(miss_neg.Theta()), STheta_mis_neg = Sin(miss_neg.Theta());
float CTheta_vis_pos = Cos(vis_pos.Theta()), STheta_vis_pos = Sin(vis_pos.Theta());
float CTheta_vis_neg = Cos(vis_neg.Theta()), STheta_vis_neg = Sin(vis_neg.Theta());
float E2Pvis_pos = Sqrt(1+vis_pos.M2()/(vis_pos.Vect()).Mag2()), E2Pvis_neg = Sqrt(1+vis_neg.M2()/(vis_neg.Vect()).Mag2());
float E2Pmis_pos = Sqrt(1+miss_pos.M2()/(miss_pos.Vect()).Mag2()), E2Pmis_neg = Sqrt(1+miss_neg.M2()/(miss_neg.Vect()).Mag2());
float denominator = (CTheta_mis_pos*E2Pvis_pos-CTheta_vis_pos*E2Pmis_pos)*(CTheta_mis_neg*E2Pvis_neg-CTheta_vis_neg*E2Pmis_neg);
if(0==denominator) return 0.;

float Ex_miss = _Ex_miss;
float Ey_miss = _Ey_miss;
float M_tau_pos = 1.777;
float M_tau_neg = 1.777;

float mass1 = vis_pos.M(), mass2= vis_neg.M();
float vis_m_pos, vis_m_neg;
if(mass2>mass1) vis_m_pos = 0.000511, vis_m_neg=0.105658;
else vis_m_neg = 0.000511, vis_m_pos=0.105658;

float vis_m_pos2 = vis_m_pos*vis_m_pos, vis_m_neg2=vis_m_neg*vis_m_neg;
float Estar_pos = 0.5*(M_tau_pos*M_tau_pos + vis_m_pos2 - miss_pos.M2())/M_tau_pos;
float Estar_neg = 0.5*(M_tau_neg*M_tau_neg + vis_m_neg2 - miss_neg.M2())/M_tau_neg;
float Pstar_pos = 0.5*Sqrt(Power(M_tau_pos*M_tau_pos - vis_m_pos2  - miss_pos.M2(),2) - 4.*vis_m_pos2*miss_pos.M2())/M_tau_pos;
float Pstar_neg = 0.5*Sqrt(Power(M_tau_neg*M_tau_neg - vis_m_neg2  - miss_neg.M2(),2) - 4.*vis_m_neg2*miss_neg.M2())/M_tau_neg;
float E_vis_pos = vis_pos.E();
float E_vis_neg = vis_neg.E();
float beta_pos = (vis_pos + miss_pos).P()/(vis_pos + miss_pos).E();
float beta_neg = (vis_neg + miss_neg).P()/(vis_neg + miss_neg).E();
float gamma_pos = 1./Sqrt(1-beta_pos*beta_pos);
float gamma_neg = 1./Sqrt(1-beta_neg*beta_neg);
float PTmis_pos = miss_pos.Pt();
float PTmis_neg = miss_neg.Pt();
float delphi_pos = vis_pos.DeltaPhi(miss_pos), delphi_neg = vis_neg.DeltaPhi(miss_neg);
float phi_pos = miss_pos.Phi(), phi_neg = miss_neg.Phi();
float cos_theta_pos = (gamma_pos*Estar_pos-E_vis_pos)/(gamma_pos*beta_pos*Pstar_pos);
float cos_theta_neg = (gamma_neg*Estar_neg-E_vis_neg)/(gamma_neg*beta_neg*Pstar_neg);


// calculate the probability for cos_theta
float M_tau4 = M_tau*M_tau*M_tau*M_tau, vis_m_pos4 = vis_m_pos2*vis_m_pos2, vis_m_neg4 = vis_m_neg2*vis_m_neg2;
float Norm_el = (8*M_tau4)/( (M_tau4-vis_m_pos4)*(M_tau4-8*M_tau*M_tau*vis_m_pos2+vis_m_pos4) + 24.*vis_m_pos4*M_tau4*Log(M_tau/vis_m_pos) );
float Norm_mu = (8*M_tau4)/( (M_tau4-vis_m_neg4)*(M_tau4-8*M_tau*M_tau*vis_m_neg2+vis_m_neg4) + 24.*vis_m_neg4*M_tau4*Log(M_tau/vis_m_neg) );

float S_tau_el = 1, S_tau_mu = 1;	// choose tau polarization
float P_cospos =Pstar_pos*(2.*M_tau*(vis_m_pos2+2.*Estar_pos*Estar_pos)+3.*Estar_pos*(M_tau*M_tau+vis_m_pos2)) - 
				Pstar_pos*Pstar_pos*S_tau_el*cos_theta_pos*(3.*vis_m_pos2+M_tau*M_tau+4.*M_tau*Estar_pos);
				P_cospos*=Norm_el*miss_pos.M();
float P_cosneg =Pstar_neg*(2.*M_tau*(vis_m_neg2+2.*Estar_neg*Estar_neg)+3.*Estar_neg*(M_tau*M_tau+vis_m_neg2)) - 
				Pstar_neg*Pstar_neg*S_tau_mu*cos_theta_neg*(3.*vis_m_neg2+M_tau*M_tau+4.*M_tau*Estar_neg);
				P_cosneg*=Norm_mu*miss_neg.M();

//calculate the Jacobian
float dLogPTmis_pos_dphi_pos = -1. / Tan(phi_neg - phi_pos);
float dLogPTmis_pos_dphi_neg = 1. / Tan(phi_neg - phi_pos) - (Ex_miss*Cos(phi_neg)+ Ey_miss*Sin(phi_neg))/(PTmis_pos*Sin(phi_neg-phi_pos));
float dLogPTmis_neg_dphi_pos = 1. / Tan(phi_pos - phi_neg) - (Ex_miss*Cos(phi_pos)+ Ey_miss*Sin(phi_pos))/(PTmis_neg*Sin(phi_pos-phi_neg));
float dLogPTmis_neg_dphi_neg = -dLogPTmis_pos_dphi_pos;

float Xterm = 	(CTheta_mis_pos*STheta_vis_pos*(dLogPTmis_pos_dphi_pos*Cos(delphi_pos) - Sin(delphi_pos)) - STheta_mis_pos*CTheta_vis_pos*dLogPTmis_pos_dphi_pos) * 
				(CTheta_mis_neg*STheta_vis_neg*(dLogPTmis_neg_dphi_neg*Cos(delphi_neg) - Sin(delphi_neg)) - STheta_mis_neg*CTheta_vis_neg*dLogPTmis_neg_dphi_neg) -
				(CTheta_mis_pos*STheta_vis_pos*Cos(delphi_pos)-STheta_mis_pos*CTheta_vis_pos) * dLogPTmis_pos_dphi_neg * 
				(CTheta_mis_neg*STheta_vis_neg*Cos(delphi_neg)-STheta_mis_neg*CTheta_vis_neg) * dLogPTmis_neg_dphi_pos;
float Prefactor = 	PTmis_pos * (E_vis_pos*gamma_pos-Estar_pos) / (Power(beta_pos*gamma_pos,3)*Pstar_pos) *
					PTmis_neg * (E_vis_neg*gamma_neg-Estar_neg) / (Power(beta_neg*gamma_neg,3)*Pstar_neg) ;

float Jacobian = Abs( Prefactor * Xterm / denominator);

return Jacobian * P_cospos * P_cosneg;	// P(delPhi,m) = P(cos_theta,m)*Jacobian
}


float mmc::Scan6dAnal(const TLorentzVector _lep1, const TLorentzVector _lep2, const float _metx, const float _mety, const float sigma)
{

  clock_t c_t0=clock(),c_t1;

	m_mz_peak=-1;
	m_mz_mean=-1;
	m_mz_maxprob=-1;
	double maxprob=0.;
    Corr_met_ex=_metx;
    Corr_met_ey=_mety;
    TH1F* h_mz_find_peak = new TH1F("h_mz_find_peak", "h_mz_find_peak", 500, 0., 500.);

//    TH1F* h_mz_Ex_peak = new TH1F("h_mz_Ex_peak", "h_mz_Ex_peak", 200, -100., 100.);
//    TH1F* h_mz_Ey_peak = new TH1F("h_mz_Ey_peak", "h_mz_Ey_peak", 200, -100., 100.);
	
//    const double sigma = m_noise + m_alpha * sqrt(_sumet);
	const double Epsilon = 1E-2;

	double vis_m_pos = _lep1.M(), vis_m_neg= _lep2.M();
	double Evis_el =  _lep1.E(), Evis_mu = _lep2.E();
	double l1=_lep1.P(), l2=_lep2.P();
	if(vis_m_pos<0) {Evis_el=Sqrt(l1*l1 + 0.000511*0.000511); vis_m_pos=0.000511;}
	if(vis_m_neg<0) {Evis_mu=Sqrt(l2*l2 + 0.000511*0.000511); vis_m_neg=0.000511;}
	double vis_m_pos2 = vis_m_pos*vis_m_pos, vis_m_neg2=vis_m_neg*vis_m_neg;
	
	double lx1 = _lep1.X(), ly1 = _lep1.Y(), lz1 = _lep1.Z();
	double lx2 = _lep2.X(), ly2 = _lep2.Y(), lz2 = _lep2.Z();
	double M_tau2=M_tau*M_tau;
	double vis_m_posOverM2 = vis_m_pos2/M_tau2, vis_m_negOverM2 = vis_m_neg2/M_tau2;

	// Constants for quartic equation of Pz1
	double inv_del = 1./(lx1*ly2 - lx2*ly1);
	double bx = -inv_del*ly2*lz1, by =  inv_del*lx2*lz1;
	double cx = -inv_del*ly1*lz2, cy =  inv_del*lx1*lz2;
	double a1 = 1. + bx*bx + by*by + cx*cx + cy*cy;
	double b2 = bx*cx+by*cy;		
	double inv_A = 1./(a1*a1 - 4.*b2*b2);

	double costheta_step = (1.+theta_star_max) / (m_costheta_6D+2.);
	double m_step = (m_mma_limhigh - m_mma_limlow) / (m_mma_n_6D+2.);
	double Z_min = Exp(-0.5*m_met_limsigma*m_met_limsigma);
	double PT_res2=0, Etau_pos=0, Etau_neg=0, P_ET_int=0.;
	double meanEx=0, meanEy=0;

// Correcting MET measuriment
//Loop 1,2 MET loop
for(double del_phi=0.1;del_phi<2.*Pi()+0.1;del_phi+=2.*Pi()/MET_scan_points_Phi){
for(double delZ=0.995;delZ>=Z_min;delZ-=(0.995-Z_min)/MET_scan_points_Z){
	double MET_counter=0;
	double Ex = _metx - Sqrt(-2*Log(delZ))*sigma*Cos(del_phi);
	double Ey = _mety - Sqrt(-2*Log(delZ))*sigma*Sin(del_phi);
	double Et_miss2 = Ex*Ex + Ey*Ey;
	PT_res2 = (lx1 + lx2 + Ex)*(lx1 + lx2 + Ex) + (ly1 + ly2 + Ey)*(ly1 + ly2 + Ey);	// Higgs transverse momentum
	
	// Loop 3,4 m_miss scan
	for (double miss_vis_m_pos = m_mma_limlow+m_step ; miss_vis_m_pos < m_mma_limhigh; miss_vis_m_pos += m_step ) {
	for (double miss_vis_m_neg = m_mma_limlow+m_step ; miss_vis_m_neg < m_mma_limhigh; miss_vis_m_neg += m_step ) {
		double mis_m_posOverM2 = miss_vis_m_pos*miss_vis_m_pos/M_tau2, mis_m_negOverM2 = miss_vis_m_neg*miss_vis_m_neg/M_tau2;
		double frac_minus_pos = vis_m_posOverM2 - mis_m_posOverM2, frac_minus_neg = vis_m_negOverM2 - mis_m_negOverM2;
		double frac_plus_pos  = vis_m_posOverM2 + mis_m_posOverM2, frac_plus_neg  = vis_m_negOverM2 + mis_m_negOverM2;
		double Pstar_pos_N = Sqrt((1.-frac_plus_pos)*(1.-frac_plus_pos) - 4.*miss_vis_m_pos*miss_vis_m_pos*vis_m_pos2/(M_tau2*M_tau2)); // Pstar*2/Mtau
		double Pstar_neg_N = Sqrt((1.-frac_plus_neg)*(1.-frac_plus_neg) - 4.*miss_vis_m_neg*miss_vis_m_neg*vis_m_neg2/(M_tau2*M_tau2)); // Pstar*2/Mtau
		double Emiss_pos_min = 0.5*M_tau*(1. - frac_minus_pos), Emiss_neg_min = 0.5*M_tau*(1. - frac_minus_neg);

// Loop 5,6 cos_theta scan
	for (double cos_el = costheta_step-1. ; cos_el < theta_star_max ; cos_el += costheta_step ) {

		double del_pos = 1. - 0.25*M_tau2/(Evis_el*Evis_el)*((1.+frac_minus_pos)*(1.+frac_minus_pos) - cos_el*cos_el*((1.-frac_plus_pos)*(1.-frac_plus_pos) - 4.*miss_vis_m_pos*miss_vis_m_pos*vis_m_pos2/(M_tau2*M_tau2)));
		if(del_pos==1) continue;
		Etau_pos = ((0.5*M_tau2/Evis_el))*((1. + frac_minus_pos + Pstar_pos_N*cos_el*Sqrt(del_pos))/(1.-del_pos));
		if( (Etau_pos<M_tau) || (Etau_pos>Evis_el/f_cut) ) continue;

	for (double cos_mu = costheta_step-1. ; cos_mu < theta_star_max ; cos_mu += costheta_step ) {

		// Solving for Emiss and Pmiss
		double del_neg = 1. - 0.25*M_tau2/(Evis_mu*Evis_mu)*((1.+frac_minus_neg)*(1.+frac_minus_neg) - cos_mu*cos_mu*((1.-frac_plus_neg)*(1.-frac_plus_neg) - 4.*miss_vis_m_neg*miss_vis_m_neg*vis_m_neg2/(M_tau2*M_tau2)));
		if(del_neg==1) continue;
		Etau_neg = ((0.5*M_tau2/Evis_mu))*((1. + frac_minus_neg + Pstar_neg_N*cos_mu*Sqrt(del_neg))/(1.-del_neg));			
		if(Etau_neg<M_tau || Etau_neg>Evis_mu/f_cut) continue;	// upper limit on the tau energy from pt cut fit
		double Emiss_pos = Etau_pos - Evis_el, Emiss_neg = Etau_neg - Evis_mu;
        if(Emiss_pos< Emiss_pos_min || Emiss_neg < Emiss_neg_min ) continue;
		double Pmiss1 = Sqrt( Emiss_pos*Emiss_pos - miss_vis_m_pos*miss_vis_m_pos ), Pmiss2 = Sqrt( Emiss_neg*Emiss_neg - miss_vis_m_neg*miss_vis_m_neg );
		
		// Solving for Pz1 and Pz2, define variables
		double alpha1 = Emiss_pos*Evis_el - 0.5*(M_tau2 - miss_vis_m_pos*miss_vis_m_pos - vis_m_pos2), alpha2 = Emiss_neg*Evis_mu - 0.5*(M_tau2 - miss_vis_m_neg*miss_vis_m_neg - vis_m_neg2);

		double ax = inv_del*((alpha1*ly2 + alpha2*ly1) - ly1*(Ex*lx2 + Ey*ly2)), ay= -inv_del*((alpha1*lx2+alpha2*lx1) - lx1*(Ex*lx2 + Ey*ly2));
		double b1 = ax*bx + ay*by + (bx*cx+by*cy)*(Ex*cx+Ey*cy) + (cx*cx+cy*cy)*(Ex*bx+Ey*by);
		double a2 = ax*cx + ay*cy + (cx*cx+cy*cy)*(Ex*cx+Ey*cy);
		double del_const = (Ex*cx+Ey*cy)*(Ex*cx+Ey*cy) - Et_miss2 - Pmiss1*Pmiss1 + Pmiss2*Pmiss2 + 2.*(Ex*ax + Ey*ay);
		double c1 = ax*ax + ay*ay - Pmiss1*Pmiss1 + (cx*cx+cy*cy)*(Pmiss2*Pmiss2-Pmiss1*Pmiss1+2.*(Ex*cx+Ey*cy)*(Ex*cx+Ey*cy) - Et_miss2 + 2.*(ax*Ex+ay*Ey)) + 2.*(ax*cx+ay*cy)*(Ex*cx+Ey*cy);

		double B = 4.*a1*b1 - 8.*b2*(a2 + b2*(Ex*bx+Ey*by));
		double C = 2.*a1*c1 + 4.*b1*b1-4.*b2*b2*del_const - 4.*a2*a2 - 16.*a2*b2*(Ex*bx+Ey*by);
		double D = 4.*b1*c1 - 8.*a2*(a2*(Ex*bx+Ey*by)+b2*del_const);
		double E = c1*c1 - 4.*a2*a2*del_const;
	
		double a=B*inv_A, b=C*inv_A, c=D*inv_A, d=E*inv_A;
		double Pz1_sol[4]={2.*Pmiss1,2.*Pmiss1,2.*Pmiss1,2.*Pmiss1}, delta_sol2[4],  Pz2_sol[4][2], Px1_sol[4][2], Py1_sol[4][2];
		bool sol_good[4]={false, false, false, false};
		bool good_Pz2[4][2] = {{false, false}, {false, false}, {false, false}, {false, false}};
		
		// solve for Pz1: Pz1^4 + a*Pz1^3 + b*Pz1^2 + c*Pz1 + d = 0
		double alpha = -3./8.*a*a + b;
		double beta = a*a*a/8. - 0.5*a*b + c;
		double gamma = -3./256.*a*a*a*a + a*a*b/16. - 0.25*a*c + d;
		bool beta_zero = false;
		if(Abs(beta)<Epsilon) beta_zero = true;
		if(beta_zero) {
			double delta_beta = alpha*alpha - 4.*gamma;
			if( delta_beta < -Epsilon ) continue;
			if( delta_beta < Epsilon ){						// true for delta_beta=0
				if(alpha>Epsilon) continue;
				if(alpha>-Epsilon) Pz1_sol[0]=-0.25*a;		// true for alpha=0
				else {Pz1_sol[0]=-0.25*a + Sqrt(-0.5*alpha); Pz1_sol[1]= -0.25*a - Sqrt(-0.5*alpha);}
			}
			else{
				double u2_1 = -0.5*alpha + 0.5*Sqrt(delta_beta), u2_2 = -0.5*alpha - 0.5*Sqrt(delta_beta);
				if(u2_1>-Epsilon) {
					if(u2_1<Epsilon) Pz1_sol[0]=-0.25*a;	// true for u2_1=0
					else{
						Pz1_sol[0]=Sqrt(u2_1) - 0.25*a; Pz1_sol[1]= -Sqrt(u2_1) - 0.25*a;
					}
				}
				if(u2_2>-Epsilon) {
					if(u2_2<Epsilon) Pz1_sol[2]=-0.25*a;	// true for u2_2=0
					else{
						Pz1_sol[2]=Sqrt(u2_2)-0.25*a; Pz1_sol[3]= -Sqrt(u2_2)-0.25*a;
					}
				}
			}	
		}
		else{
			double v, V, U;
			double third = 1./3.;
			double P = -alpha*alpha/12. - gamma, Q = -alpha*alpha*alpha/108. + alpha*gamma/3. - beta*beta/8.;
			double Qsign=1;
			if(Q<0) Qsign=-1.;
			double QPdel = 0.25*Q*Q + P*P*P/27.;
			if( QPdel<-Epsilon ) continue;
			if( QPdel< Epsilon ) U=-1.*Qsign*Power(0.5*Qsign*Q,third);
			else {
				double R = -0.5*Q + Sqrt(QPdel);
				double Rsign = 1;
				if(R<0) Rsign = -1.;
				U = Rsign*Power(Abs(R),third);		
			}
			if(U) V = -P/U*third;
			else V = -Qsign*Power(Qsign*Q,third);
			v = U + V;
			double y = -5./6.*alpha + U + V;
			double W2 = (alpha + 2.*y), W=0.;
			if(W2>Epsilon)
				W = Sqrt(W2);
			if(W2<-Epsilon) continue;
			double in_qsuare_Plus = -(3.*alpha + 2.*y + 2*beta/W);
			double in_qsuare_Minus = -(3.*alpha + 2.*y - 2*beta/W);
			if(in_qsuare_Plus>=0){
				Pz1_sol[0] = -0.25*a + 0.5*(W - Sqrt(in_qsuare_Plus));
				Pz1_sol[1] = -0.25*a + 0.5*(W + Sqrt(in_qsuare_Plus));
			}
			if(in_qsuare_Minus>=0){
				Pz1_sol[2] = -0.25*a + 0.5*(-W - Sqrt(in_qsuare_Minus));
				Pz1_sol[3] = -0.25*a + 0.5*(-W + Sqrt(in_qsuare_Minus));
			}		
		}
                double ResiduePz1[4]={999., 999., 999., 999.};
                double ResiduePz2[4][2]={{999., 999.}, {999., 999.}, {999., 999.}, {999., 999.}};
                double MinResidue=999.;
                //solve for Pz2, and {Px1, Py1}
                for ( int i =0 ; i < 4 ; i ++ ){
                        double Pcos_theta1 = Pz1_sol[i]/Pmiss1;
                        if(Abs(Pcos_theta1)>1.) continue;
                        ResiduePz1[i]=Abs(Pz1_sol[i]*Pz1_sol[i]*Pz1_sol[i]*Pz1_sol[i] + a*Pz1_sol[i]*Pz1_sol[i]*Pz1_sol[i] + b*Pz1_sol[i]*Pz1_sol[i]+c*Pz1_sol[i]+d);
                        sol_good[i]=true;
                        if(!sol_good[i]) continue;
                        //  PT1_sol[i]=Sqrt(Pmiss1*Pmiss1-Pz1_sol[i]*Pz1_sol[i]);
                        delta_sol2[i] = (Ex*cx+Ey*cy)*(Ex*cx+Ey*cy) - Et_miss2 + Pz1_sol[i]*Pz1_sol[i] - Pmiss1*Pmiss1 + Pmiss2*Pmiss2 + 2.*Ex*(ax+bx*Pz1_sol[i]) + 2.*Ey*(ay+by*Pz1_sol[i]);
                        if(delta_sol2[i]<-10.*Epsilon) {sol_good[i]=false; continue;}
                        if(delta_sol2[i]<10.*Epsilon) {Pz2_sol[i][0]=Ex*cx + Ey*cy; Pz2_sol[i][1]=2.*Pmiss2;}
                        else {Pz2_sol[i][0] = Ex*cx + Ey*cy + Sqrt(delta_sol2[i]), Pz2_sol[i][1] = Ex*cx + Ey*cy - Sqrt(delta_sol2[i]);}        // solution for Pz2
                        for (int j=0; j<2; ++j){
                                double Pcos_theta2 = Pz2_sol[i][j]/Pmiss2;
                                if(Abs(Pcos_theta2)>1.) continue;
                                //PT2_sol[i][j] = Sqrt(Pmiss2*Pmiss2 - Pz2_sol[i][j]*Pz2_sol[i][j]);
                                Px1_sol[i][j] =  ax + bx*Pz1_sol[i] + cx*Pz2_sol[i][j], Py1_sol[i][j] =  ay + by*Pz1_sol[i] + cy*Pz2_sol[i][j]; // solution for Px1 and Py1
                                double Pmiss12 = Px1_sol[i][j]*Px1_sol[i][j] + Py1_sol[i][j]*Py1_sol[i][j] + Pz1_sol[i]*Pz1_sol[i];
                                double Pmiss22 = (Ex-Px1_sol[i][j])*(Ex-Px1_sol[i][j]) + (Ey-Py1_sol[i][j])*(Ey-Py1_sol[i][j]) + Pz2_sol[i][j]*Pz2_sol[i][j];
                                double Plcosa1 = lx1*Px1_sol[i][j] + ly1*Py1_sol[i][j] + lz1*Pz1_sol[i];
                                double Plcosa2 = lx2*(Ex-Px1_sol[i][j]) + ly2*(Ey-Py1_sol[i][j]) + lz2*Pz2_sol[i][j];
                                // write physical solutions
                                ResiduePz2[i][j]=Abs(Pmiss1*Pmiss1-Pmiss12) + Abs(Pmiss2*Pmiss2-Pmiss22) + Abs(alpha1-Plcosa1) + Abs(alpha2-Plcosa2);
                                good_Pz2[i][j]=true;
                        }
                }
                int count_good = 0;
                double Mhiggs = 0;
                for ( int i =0 ; i < 4 ; i ++ ){
                        for (int j=0; j<2; ++j){
                                if(sol_good[i]&&good_Pz2[i][j]){
                                count_good++;
                                double mres2 = (Etau_pos+Etau_neg)*(Etau_pos+Etau_neg) - PT_res2 - (lz1 + lz2 + Pz1_sol[i] + Pz2_sol[i][j])*(lz1 + lz2 + Pz1_sol[i] + Pz2_sol[i][j]);
                                //if(mres2>0) Mhiggs+= Sqrt(mres2);
                                if(ResiduePz1[i]+ResiduePz2[i][j]<MinResidue && mres2>0 ) {MinResidue =  ResiduePz1[i]+ResiduePz2[i][j]; Mhiggs=Sqrt(mres2);}
                                }
                        }
                }
		if(!count_good) continue;
		//Mhiggs = Mhiggs/count_good;		
		if (Mhiggs<20. || Mhiggs>500.) continue;
		
		// Calculate the probability for P(cos_theta,m2nu) (from matrix element) with cos\theta* ptcut correction

        double x1=miss_vis_m_pos/M_tau, x2=miss_vis_m_neg/M_tau;
        double x1_2=x1*x1, x2_2=x2*x2;

        double Prob_el = x1*(1.-x1_2)*( 1. + x1_2 - 2.*x1_2*x1_2)*(1. - f_cut/(1-x1_2))*(4.-5.*cos_el);
        double Prob_mu = x2*(1.-x2_2)*( 1. + x2_2 - 2.*x2_2*x2_2)*(1. - f_cut/(1-x2_2))*(4.-5.*cos_mu);

        double Probability = Prob_el*Prob_mu;
		
		MET_counter+=Probability;		
		h_mz_find_peak->Fill(Mhiggs,Probability);
		if(maxprob<Probability){maxprob=Probability; m_mz_maxprob=Mhiggs;}	
		
	    } // theta mu
		} // theta el
	} // m_miss mu
    } // m_miss el
	P_ET_int+=MET_counter;
	//h_mz_Ex_peak->Fill(_metx-Ex,MET_counter);
	//h_mz_Ey_peak->Fill(_mety-Ey,MET_counter);
	if(MET_counter){
		meanEx+=MET_counter*Ex;
		meanEy+=MET_counter*Ey;
	}	

} // MET_Z
} // MET_Phi

Corr_met_ex=meanEx/P_ET_int;
Corr_met_ey=meanEy/P_ET_int;

/*
 if( (h_mz_Ex_peak->GetEntries()) && (h_mz_Ey_peak->GetEntries()) ) {
   int binPeak_ex = h_mz_Ex_peak->GetMaximumBin();
   int binPeak_ey = h_mz_Ey_peak->GetMaximumBin();
   double weightsum_x = 0;
   double currWeight_x = 0;
   double currValue_x = 0;
   double numerator_x = 0;
   double weightsum_y = 0;
   double currWeight_y = 0;
   double currValue_y = 0;
   double numerator_y = 0;
   for (int i = -3; i <= 3; i++) {
     currValue_x = h_mz_Ex_peak->GetBinCenter(binPeak_ex + i);
     currWeight_x = h_mz_Ex_peak->GetBinContent(binPeak_ex + i);
     weightsum_x += currWeight_x;
     numerator_x += currWeight_x * currValue_x;
     currValue_y = h_mz_Ey_peak->GetBinCenter(binPeak_ey + i);
     currWeight_y = h_mz_Ey_peak->GetBinContent(binPeak_ey + i);
     weightsum_y += currWeight_y;
     numerator_y += currWeight_y * currValue_y;
   }
   double peak_ex = numerator_x / weightsum_x;
   double peak_ey = numerator_y / weightsum_y;

   Corr_met_ex = _metx - peak_ex; 
   Corr_met_ey = _mety - peak_ey; 
   //   cout << "   RESULT: peak_ex " << peak_ex << " \t peak_ey " << peak_ey << endl;

 }
*/

    // Rebin the histogram for better estimation:
	//int NewBin = 500/(3.*h_mz_find_peak->GetRMS());
	//NewBin = Max(1,NewBin);
	//h_mz_find_peak->Rebin(NewBin);

	
  if(h_mz_find_peak->GetEntries()){
	  this->SetMZ_mean(h_mz_find_peak->GetMean());
	  this->SetMZ_peak(h_mz_find_peak->GetBinCenter(h_mz_find_peak->GetMaximumBin()));
  }
  this->SetMZ_maxprob(m_mz_maxprob);
  
//  delete h_mz_Ex_peak;
//  delete h_mz_Ey_peak;
  delete h_mz_find_peak;

  c_t1=clock()-c_t0;
  m_exectime=c_t1/(1.0*CLOCKS_PER_SEC);
   
  return 1.;
}

float mmc::Scan4dAnal(const TLorentzVector _lep1, const TLorentzVector _lep2, const float _metx, const float _mety)
{

  clock_t c_t0=clock(),c_t1;

	m_mz_peak=-1;
	m_mz_mean=-1;
	m_mz_maxprob=-1;
	double maxprob=0.;
  
    TH1F* h_mz_find_peak = new TH1F("h_mz_find_peak", "h_mz_find_peak", 500, 0., 500.);

	const double Epsilon = 1E-2;

	double vis_m_pos = _lep1.M(), vis_m_neg= _lep2.M();
	double Evis_el =  _lep1.E(), Evis_mu = _lep2.E();
	double l1=_lep1.P(), l2=_lep2.P();
	if(vis_m_pos<0) {Evis_el=Sqrt(l1*l1 + 0.000511*0.000511); vis_m_pos=0.000511;}
	if(vis_m_neg<0) {Evis_mu=Sqrt(l2*l2 + 0.000511*0.000511); vis_m_neg=0.000511;}
	double vis_m_pos2 = vis_m_pos*vis_m_pos, vis_m_neg2=vis_m_neg*vis_m_neg;
	
	double lx1 = _lep1.X(), ly1 = _lep1.Y(), lz1 = _lep1.Z();
	double lx2 = _lep2.X(), ly2 = _lep2.Y(), lz2 = _lep2.Z();
	double M_tau2=M_tau*M_tau;
	double vis_m_posOverM2 = vis_m_pos2/M_tau2, vis_m_negOverM2 = vis_m_neg2/M_tau2;

	double costheta_step = (1.+theta_star_max)/ (m_costheta_4D+2.);
	double m_step = (m_mma_limhigh - m_mma_limlow) / (m_mma_n_4D+2.);
	double PT_res2, Res_mass=0., Etau_pos, Etau_neg;

	double Ex = _metx;
	double Ey = _mety;
	double Et_miss2 = Ex*Ex + Ey*Ey;
	PT_res2 = (lx1 + lx2 + Ex)*(lx1 + lx2 + Ex) + (ly1 + ly2 + Ey)*(ly1 + ly2 + Ey);
	
	// Constants for quartic equation of Pz1
	double inv_del = 1./(lx1*ly2 - lx2*ly1);
	double bx = -inv_del*ly2*lz1, by =  inv_del*lx2*lz1;
	double cx = -inv_del*ly1*lz2, cy =  inv_del*lx1*lz2;
	double a1 = 1. + bx*bx + by*by + cx*cx + cy*cy;
	double b2 = bx*cx+by*cy;		
	double inv_A = 1./(a1*a1 - 4.*b2*b2);	
	
// Loop 1,2 m_miss scan
	for (double miss_vis_m_pos = m_mma_limlow+m_step ; miss_vis_m_pos < m_mma_limhigh; miss_vis_m_pos += m_step ) {
	for (double miss_vis_m_neg = m_mma_limlow+m_step ; miss_vis_m_neg < m_mma_limhigh; miss_vis_m_neg += m_step ) {
		double mis_m_posOverM2 = miss_vis_m_pos*miss_vis_m_pos/M_tau2, mis_m_negOverM2 = miss_vis_m_neg*miss_vis_m_neg/M_tau2;
		double frac_minus_pos = vis_m_posOverM2 - mis_m_posOverM2, frac_minus_neg = vis_m_negOverM2 - mis_m_negOverM2;
		double frac_plus_pos  = vis_m_posOverM2 + mis_m_posOverM2, frac_plus_neg  = vis_m_negOverM2 + mis_m_negOverM2;
		double Emiss_pos_min = 0.5*M_tau*(1. - frac_minus_pos), Emiss_neg_min = 0.5*M_tau*(1. - frac_minus_neg);

// Loop 5,6 cos_theta scan
	for (double cos_el = costheta_step-1. ; cos_el < theta_star_max ; cos_el += costheta_step ) {

		double del_pos = 1. - 0.25*M_tau2/(Evis_el*Evis_el)*((1.+frac_minus_pos)*(1.+frac_minus_pos) - cos_el*cos_el*((1.-frac_plus_pos)*(1.-frac_plus_pos) - 4.*miss_vis_m_pos*miss_vis_m_pos*vis_m_pos2/(M_tau2*M_tau2)));
		if(del_pos==1) continue;
		Etau_pos = ((0.5*M_tau2/Evis_el))*((1. + frac_minus_pos + Sqrt((1.-frac_plus_pos)*(1.-frac_plus_pos) - 4.*miss_vis_m_pos*miss_vis_m_pos*vis_m_pos2/(M_tau2*M_tau2))*cos_el*Sqrt(del_pos))/(1.-del_pos));
		if( (Etau_pos<M_tau) || (Etau_pos>Evis_el/f_cut) ) continue;

	for (double cos_mu = costheta_step-1.; cos_mu < theta_star_max ; cos_mu += costheta_step ) {

		// Solving for Emiss and Pmiss
		double del_neg = 1. - 0.25*M_tau2/(Evis_mu*Evis_mu)*((1.+frac_minus_neg)*(1.+frac_minus_neg) - cos_mu*cos_mu*((1.-frac_plus_neg)*(1.-frac_plus_neg) - 4.*miss_vis_m_neg*miss_vis_m_neg*vis_m_neg2/(M_tau2*M_tau2)));
		if(del_neg==1) continue;
		Etau_neg = ((0.5*M_tau2/Evis_mu))*((1. + frac_minus_neg + Sqrt((1.-frac_plus_neg)*(1.-frac_plus_neg) - 4.*miss_vis_m_neg*miss_vis_m_neg*vis_m_neg2/(M_tau2*M_tau2))*cos_mu*Sqrt(del_neg))/(1.-del_neg));			
		if(Etau_neg<M_tau || Etau_neg>Evis_mu/f_cut) continue;	// upper limit on the tau energy from pt cut fit
		double Emiss_pos = Etau_pos - Evis_el, Emiss_neg = Etau_neg - Evis_mu;
        if(Emiss_pos< Emiss_pos_min || Emiss_neg < Emiss_neg_min ) continue;
		double Pmiss1 = Sqrt( Emiss_pos*Emiss_pos - miss_vis_m_pos*miss_vis_m_pos ), Pmiss2 = Sqrt( Emiss_neg*Emiss_neg - miss_vis_m_neg*miss_vis_m_neg );
		
		// Solving for Pz1 and Pz2, define variables
		double alpha1 = Emiss_pos*Evis_el - 0.5*(M_tau2 - miss_vis_m_pos*miss_vis_m_pos - vis_m_pos2), alpha2 = Emiss_neg*Evis_mu - 0.5*(M_tau2 - miss_vis_m_neg*miss_vis_m_neg - vis_m_neg2);

		double ax = inv_del*((alpha1*ly2 + alpha2*ly1) - ly1*(Ex*lx2 + Ey*ly2)), ay= -inv_del*((alpha1*lx2+alpha2*lx1) - lx1*(Ex*lx2 + Ey*ly2));
		double b1 = ax*bx + ay*by + (bx*cx+by*cy)*(Ex*cx+Ey*cy) + (cx*cx+cy*cy)*(Ex*bx+Ey*by);
		double a2 = ax*cx + ay*cy + (cx*cx+cy*cy)*(Ex*cx+Ey*cy);
		double del_const = (Ex*cx+Ey*cy)*(Ex*cx+Ey*cy) - Et_miss2 - Pmiss1*Pmiss1 + Pmiss2*Pmiss2 + 2.*(Ex*ax + Ey*ay);
		double c1 = ax*ax + ay*ay - Pmiss1*Pmiss1 + (cx*cx+cy*cy)*(Pmiss2*Pmiss2-Pmiss1*Pmiss1+2.*(Ex*cx+Ey*cy)*(Ex*cx+Ey*cy) - Et_miss2 + 2.*(ax*Ex+ay*Ey)) + 2.*(ax*cx+ay*cy)*(Ex*cx+Ey*cy);

		double B = 4.*a1*b1 - 8.*b2*(a2 + b2*(Ex*bx+Ey*by));
		double C = 2.*a1*c1 + 4.*b1*b1-4.*b2*b2*del_const - 4.*a2*a2 - 16.*a2*b2*(Ex*bx+Ey*by);
		double D = 4.*b1*c1 - 8.*a2*(a2*(Ex*bx+Ey*by)+b2*del_const);
		double E = c1*c1 - 4.*a2*a2*del_const;
	
		double a=B*inv_A, b=C*inv_A, c=D*inv_A, d=E*inv_A;
		double Pz1_sol[4]={2.*Pmiss1,2.*Pmiss1,2.*Pmiss1,2.*Pmiss1}, delta_sol2[4],  Pz2_sol[4][2], Px1_sol[4][2], Py1_sol[4][2];
		bool sol_good[4]={false, false, false, false};
		bool good_Pz2[4][2] = {{false, false}, {false, false}, {false, false}, {false, false}};

		
		// solve for Pz1: Pz1^4 + a*Pz1^3 + b*Pz1^2 + c*Pz1 + d = 0
		double alpha = -3./8.*a*a + b;
		double beta = a*a*a/8. - 0.5*a*b + c;
		double gamma = -3./256.*a*a*a*a + a*a*b/16. - 0.25*a*c + d;
		bool beta_zero = false;
		if(Abs(beta)<Epsilon) beta_zero = true;
		if(beta_zero) {
			double delta_beta = alpha*alpha - 4.*gamma;
			if( delta_beta < -Epsilon ) continue;
			if( delta_beta < Epsilon ){						// true for delta_beta=0
				if(alpha>Epsilon) continue;
				if(alpha>-Epsilon) Pz1_sol[0]=-0.25*a;		// true for alpha=0
				else {Pz1_sol[0]= -0.25*a + Sqrt(-0.5*alpha); Pz1_sol[1]= -0.25*a - Sqrt(-0.5*alpha);}
			}
			else{
				double u2_1 = -0.5*alpha + 0.5*Sqrt(delta_beta), u2_2 = -0.5*alpha - 0.5*Sqrt(delta_beta);
				if(u2_1>-Epsilon) {
					if(u2_1<Epsilon) Pz1_sol[0]=-0.25*a;	// true for u2_1=0
					else{
						Pz1_sol[0]= Sqrt(u2_1) - 0.25*a; Pz1_sol[1]= -Sqrt(u2_1) - 0.25*a;
					}
				}
				if(u2_2>-Epsilon) {
					if(u2_2<Epsilon) Pz1_sol[2]=-0.25*a;
					else{
						Pz1_sol[2]= Sqrt(u2_2)-0.25*a; Pz1_sol[3]= -Sqrt(u2_2)-0.25*a;
					}
				}
			}	
		}
		else{
			double v, V, U;
			double third = 1./3.;
			double P = -alpha*alpha/12. - gamma, Q = -alpha*alpha*alpha/108. + alpha*gamma/3. - beta*beta/8.;
			double Qsign=1;
			if(Q<0) Qsign=-1.;
			double QPdel = 0.25*Q*Q + P*P*P/27.;
			if( QPdel<-Epsilon ) continue;
			if( QPdel< Epsilon ) U=-1.*Qsign*Power(0.5*Qsign*Q,third);
			else {
				double R = -0.5*Q + Sqrt(QPdel);
				double Rsign = 1;
				if(R<0) Rsign = -1.;
				U = Rsign*Power(Abs(R),third);		
			}
			if(U) V = -P/U*third;
			else V = -Qsign*Power(Qsign*Q,third);
			v = U + V;
			double y = -5./6.*alpha + U + V;
			double W2 = (alpha + 2.*y), W=0.;
			if(W2>Epsilon)
				W = Sqrt(W2);
			if(W2<-Epsilon) continue;
			double in_qsuare_Plus = -(3.*alpha + 2.*y + 2*beta/W);
			double in_qsuare_Minus = -(3.*alpha + 2.*y - 2*beta/W);
			if(in_qsuare_Plus>=0){
				Pz1_sol[0] = -0.25*a + 0.5*(W - Sqrt(in_qsuare_Plus));
				Pz1_sol[1] = -0.25*a + 0.5*(W + Sqrt(in_qsuare_Plus));
			}
			if(in_qsuare_Minus>=0){
				Pz1_sol[2] = -0.25*a + 0.5*(-W - Sqrt(in_qsuare_Minus));
				Pz1_sol[3] = -0.25*a + 0.5*(-W + Sqrt(in_qsuare_Minus));
			}		
		}

                double ResiduePz1[4]={999., 999., 999., 999.};
                double ResiduePz2[4][2]={{999., 999.}, {999., 999.}, {999., 999.}, {999., 999.}};
                double MinResidue=999.;
		//solve for Pz2, and {Px1, Py1}
                for ( int i =0 ; i < 4 ; i ++ ){
                        double Pcos_theta1 = Pz1_sol[i]/Pmiss1;
                        if(Abs(Pcos_theta1)>1.) continue;
                        ResiduePz1[i]=Abs(Pz1_sol[i]*Pz1_sol[i]*Pz1_sol[i]*Pz1_sol[i] + a*Pz1_sol[i]*Pz1_sol[i]*Pz1_sol[i] + b*Pz1_sol[i]*Pz1_sol[i]+c*Pz1_sol[i]+d);
                        sol_good[i]=true;
                        if(!sol_good[i]) continue;
                        delta_sol2[i] = (Ex*cx+Ey*cy)*(Ex*cx+Ey*cy) - Et_miss2 + Pz1_sol[i]*Pz1_sol[i] - Pmiss1*Pmiss1 + Pmiss2*Pmiss2 + 2.*Ex*(ax+bx*Pz1_sol[i]) + 2.*Ey*(ay+by*Pz1_sol[i]);
                        if(delta_sol2[i]<-10.*Epsilon) {sol_good[i]=false; continue;}
                        if(delta_sol2[i]<10.*Epsilon) {Pz2_sol[i][0]=Ex*cx + Ey*cy; Pz2_sol[i][1]=2.*Pmiss2;}
                        else {Pz2_sol[i][0] = Ex*cx + Ey*cy + Sqrt(delta_sol2[i]), Pz2_sol[i][1] = Ex*cx + Ey*cy - Sqrt(delta_sol2[i]);}        // solution for Pz2
                        for (int j=0; j<2; ++j){
                                double Pcos_theta2 = Pz2_sol[i][j]/Pmiss2;
                                if(Abs(Pcos_theta2)>1.) continue;
                                Px1_sol[i][j] =  ax + bx*Pz1_sol[i] + cx*Pz2_sol[i][j], Py1_sol[i][j] =  ay + by*Pz1_sol[i] + cy*Pz2_sol[i][j]; // solution for Px1 and Py1
                                double Pmiss12 = Px1_sol[i][j]*Px1_sol[i][j] + Py1_sol[i][j]*Py1_sol[i][j] + Pz1_sol[i]*Pz1_sol[i];
                                double Pmiss22 = (Ex-Px1_sol[i][j])*(Ex-Px1_sol[i][j]) + (Ey-Py1_sol[i][j])*(Ey-Py1_sol[i][j]) + Pz2_sol[i][j]*Pz2_sol[i][j];
                                double Plcosa1 = lx1*Px1_sol[i][j] + ly1*Py1_sol[i][j] + lz1*Pz1_sol[i];
                                double Plcosa2 = lx2*(Ex-Px1_sol[i][j]) + ly2*(Ey-Py1_sol[i][j]) + lz2*Pz2_sol[i][j];
                                // write physical solutions
                                ResiduePz2[i][j]=Abs(Pmiss1*Pmiss1-Pmiss12) + Abs(Pmiss2*Pmiss2-Pmiss22) + Abs(alpha1-Plcosa1) + Abs(alpha2-Plcosa2);
                                good_Pz2[i][j]=true;
                        }
                }
                int count_good = 0;
                double Mhiggs = 0;
                for ( int i =0 ; i < 4 ; i ++ ){
                        for (int j=0; j<2; ++j){
                                if(sol_good[i]&&good_Pz2[i][j]){
                                	count_good++;
                                	double mres2 = (Etau_pos+Etau_neg)*(Etau_pos+Etau_neg) - PT_res2 - (lz1 + lz2 + Pz1_sol[i] + Pz2_sol[i][j])*(lz1 + lz2 + Pz1_sol[i] + Pz2_sol[i][j]);
                                	if(ResiduePz1[i]+ResiduePz2[i][j]<MinResidue && mres2>0 ) {
						MinResidue =  ResiduePz1[i]+ResiduePz2[i][j]; Mhiggs=Sqrt(mres2);
					}	
                                }
                        }
                }

		if(!count_good) continue;
		
		Res_mass = Mhiggs;

		if (Res_mass<20. || Res_mass>500.) continue;	

		// Calculate the probability for P(cos_theta,m2nu) (from matrix element) with cos\theta* ptcut correction

        double x1=miss_vis_m_pos/M_tau, x2=miss_vis_m_neg/M_tau;
        double x1_2=x1*x1, x2_2=x2*x2;

        double Prob_el = x1*(1.-x1_2)*( 1. + x1_2 - 2.*x1_2*x1_2)*(1. - f_cut/(1-x1_2))*(4.-5.*cos_el);
        double Prob_mu = x2*(1.-x2_2)*( 1. + x2_2 - 2.*x2_2*x2_2)*(1. - f_cut/(1-x2_2))*(4.-5.*cos_mu);

        double Probability = Prob_el*Prob_mu;
		
		if(maxprob<Probability){maxprob=Probability; m_mz_maxprob=Res_mass;}
			
// Fill the mass histogramm
		h_mz_find_peak->Fill(Res_mass,Probability);
		
	    } // theta mu
		} // theta el
	} // m_miss mu
    } // m_miss el


	// Rebin the histogram for better estimation:
	//int NewBin = 500/(3.*h_mz_find_peak->GetRMS());
	//NewBin = Max(1,NewBin);
	//h_mz_find_peak->Rebin(NewBin);
	
  if(h_mz_find_peak->GetEntries()){
	  this->SetMZ_mean(h_mz_find_peak->GetMean());
	  this->SetMZ_peak(h_mz_find_peak->GetBinCenter(h_mz_find_peak->GetMaximumBin()));
  }
  this->SetMZ_maxprob(m_mz_maxprob);
  
  delete h_mz_find_peak;

  c_t1=clock()-c_t0;
  m_exectime=c_t1/(1.0*CLOCKS_PER_SEC);
   
  return 1.;
}
