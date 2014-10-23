/*
 This file is part of OpenMalaria.
 
 Copyright (C) 2005-2014 Swiss Tropical and Public Health Institute
 Copyright (C) 2005-2014 Liverpool School Of Tropical Medicine
 
 OpenMalaria is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or (at
 your option) any later version.
 
 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef Hmod_MolineauxInfectionSuite
#define Hmod_MolineauxInfectionSuite

#include <cxxtest/TestSuite.h>
#include "UnittestUtil.h"
#include "ExtraAsserts.h"
#include "WithinHost/Infection/MolineauxInfection.h"
#include "util/random.h"
#include <limits>
#include <fstream>
#include <iomanip>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_statistics_double.h>

using namespace OM::WithinHost;

class MolineauxInfectionSuite : public CxxTest::TestSuite
{
public:
    void setUp () {
        util::random::seed( 1095 );     // make sure other tests don't influence our random numbers
        UnittestUtil::initTime(1);
        UnittestUtil::Infection_init_latentP_and_NaN ();
        // test should call molInit( ... )
    }
    void molInit( const std::string& mode, bool repl_gamma ){
        UnittestUtil::MolineauxWHM_setup( mode, repl_gamma );
        // Set parameters; all of these were estimated externally from OpenMalaria
        scnXml::Parameters xmlParams(0,0,"" /*interval, seed, latentP: but we don't need to set them now*/);
        xmlParams.getParameter().push_back( scnXml::Parameter( "Molineaux first local max density mean", 34, 4.7601 ) );
        xmlParams.getParameter().push_back( scnXml::Parameter( "Molineaux first local max density sd", 35, 0.5008 ) );
        xmlParams.getParameter().push_back( scnXml::Parameter( "Diff positive days mean", 36, 2.2736 ) );
        xmlParams.getParameter().push_back( scnXml::Parameter( "Diff positive days sd", 37, 0.2315 ) );
        Parameters params(xmlParams);
        MolineauxInfection::init(params);
    }
    void tearDown () {
        util::random::seed(0);  // make sure nothing else uses this seed/reports
    }
    
    static void readVector(std::vector<double>& vec, const char* file){
        ifstream istr(file);
        ETS_ASSERT( istr.is_open() );
        double val;
        while( istr >> val ){
            vec.push_back( val );
        }
        ETS_ASSERT( istr.eof() );
    }
    
    void testDensities(){
        molInit( "pairwise", false );
        
        // read densities: these were simply generated by this code, so the
        // only useful thing this test does is spot changes in output!
        vector<double> dens;
        readVector(dens,"MolineauxCirDens.txt");
        
        // pkpdID (value) isn't important since we're not using drug model here:
        MolineauxInfection* infection = new MolineauxInfection (0xFFFFFFFF);
        bool extinct = false;
        size_t day=0;
        SimTime now = sim::ts0();
        do{
            extinct = infection->update(1.0 /*no external immunity*/, now);
            SimTime age = now - infection->m_startDate - infection->latentP;
            if( age >= sim::zero() ){
                ETS_ASSERT_LESS_THAN( day, dens.size() );
                TS_ASSERT_APPROX( infection->getDensity(), dens[day] );
                day += 1;
            }
            now += sim::oneDay();
        }while(!extinct);
        TS_ASSERT_EQUALS( day, dens.size() );
        delete infection;
    }
    
    void testMolOrig(){
        molInit( "original", false );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). These stats look similar to and possibly better than those
        // from the pairwise model, when compared to those in the paper.
        stats.compare( "MolineauxStatsOrig" );
    }
    
    void testMolOrigRG(){
        molInit( "original", true );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). These stats look similar to and possibly better than those
        // from the pairwise model, when compared to those in the paper.
        stats.compare( "MolineauxStatsOrigRG" );
    }
    
    void dont_testMol1MG(){
        molInit( "1st_max_gamma", false );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). Output is nowhere near what we want.
        stats.write( "MolineauxStats1MG" );
    }
    
    void dont_testMol1MGRG(){
        molInit( "1st_max_gamma", true );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). Output is nowhere near what we want.
        stats.write( "MolineauxStats1MGRG" );
    }
    
    void dont_testMolMDG(){
        molInit( "mean_dur_gamma", false );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). Model output contains NaNs.
        stats.write( "MolineauxStatsMDG" );
    }
    
    void dont_testMolMDGRG(){
        molInit( "mean_dur_gamma", true );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). Model output contains NaNs.
        stats.write( "MolineauxStatsMDGRG" );
    }
    
    void dont_testMol1MGMDG(){
        molInit( "1st_max_and_mean_dur_gamma", false );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). Model output contains NaNs.
        stats.write( "MolineauxStats1MGMDG" );
    }
    
    void dont_testMol1MGMDGRG(){
        molInit( "1st_max_and_mean_dur_gamma", true );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). Model output contains NaNs.
        stats.write( "MolineauxStats1MGMDGRG" );;
    }
    
    void testMolPairwise(){
        molInit( "pairwise", false );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). Compared to those in the paper, these stats match the
        // first peak and prop_pos_1st reasonably well, but the interval stats,
        // prop_pos_2nd and last_pos_day don't match well.
        stats.compare( "MolineauxStatsPairwise" );
    }
    
    void testMolPairwiseRG(){
        molInit( "pairwise", true );
        MolInfStats stats( 200 );
        stats.capture();
        // We compare against model outputs (checking for changes, rather than
        // accuracy). This one compares a little more favourably tho the stats
        // in the paper than without replication gamma, though only a little.
        stats.compare( "MolineauxStatsPairwiseRG" );
    }

private:
    /** Calculates some key stats — these correspond to table 1 from the
     * Molineaux paper. Note that 'log' means 'log base 10'. */
    class MolInfStats{
        vector<double> init_slope;      // slope of a linear regression line through
        // the log densities from first positive to first local maxima
        vector<double> log_1st_max;     // log of first local maxima
        vector<int> no_max;  // number of local maxima
        vector<double> slope_max;       // slope of linear regression line through log
        // densities of local maxima
        vector<double> GM_interv;       // geometric mean of intervals between
        // consecutive local maxima
        vector<double> SD_log;  // standard deviation of logs of intervals between
        // consecutive local maxima
        vector<double> prop_pos_1st;    // proportion of observations during the first
        // half of the interval between first and last positive days which are
        // positive
        vector<double> prop_pos_2nd;    // as above, but for second half
        vector<double> last_pos_day;    // difference between first and last positive days
        
    public:
        MolInfStats(size_t N) :
            init_slope(N, numeric_limits<double>::quiet_NaN()),
            log_1st_max(N, numeric_limits<double>::quiet_NaN()),
            no_max(N, 0),
            slope_max(N, numeric_limits<double>::quiet_NaN()),
            GM_interv(N, numeric_limits<double>::quiet_NaN()),
            SD_log(N, numeric_limits<double>::quiet_NaN()),
            prop_pos_1st(N, numeric_limits<double>::quiet_NaN()),
            prop_pos_2nd(N, numeric_limits<double>::quiet_NaN()),
            last_pos_day(N, numeric_limits<double>::quiet_NaN())
        {}
        
        /** Calculate stats */
        void calc( size_t n, const vector<double>& dens ){
            size_t first_pos = 0, last_pos = 0, posFirstLocalMax = 0;
            vector<double> maxima_t, maxima_ld;
            
            // Iterate with a step of two. Note that in one case we coincide
            // with density updates, in the other we get the interpolated
            // values.
            const size_t start = 0 /* 0 or 1 */, step = 2;
            for( size_t day = start; day < dens.size(); day += step ){
                if( first_pos == 0 && dens[day] > 0.0 ) first_pos = day;
                if( dens[day] > 0.0 ) last_pos = day;    // gets re-set until end of infection
                if( day >= step && day + step < dens.size() && dens[day] > dens[day-step] && dens[day] > dens[day+step] ){
                    //NOTE: assumes non-zero densities never exactly repeat
                    maxima_t.push_back( day );
                    maxima_ld.push_back( log10(dens[day]) );
                    if( posFirstLocalMax == 0 ){
                        posFirstLocalMax = day;
                    }
                }
            }
            last_pos_day[n] = last_pos - first_pos;
            no_max[n] = maxima_t.size();
            if( maxima_t.size() == 0 ) return;  // no local maxima — shouldn't happen
            log_1st_max[n] = maxima_ld[0];
            
            size_t len_init = (maxima_t[0] - first_pos)/step + 1;
            vector<double> init_t_logdens( len_init * 2, 0 );
            for( size_t i = 0; i < len_init; i++ ){
                size_t day = step*i + first_pos;
                init_t_logdens[2*i] = day;
                init_t_logdens[2*i+1] = log10(dens[day]);
            }
            
            double c0, c1, cov00, cov01, cov11, sum_sq;
            gsl_fit_linear( &init_t_logdens[0], 2, &init_t_logdens[1], 2, len_init,
                            &c0, &c1, &cov00, &cov01, &cov11, &sum_sq );
            init_slope[n] = c1;
            
            gsl_fit_linear( &maxima_t[0], 1, &maxima_ld[0], 1, maxima_t.size(),
                            &c0, &c1, &cov00, &cov01, &cov11, &sum_sq );
            slope_max[n] = c1;
            
            double gm = 1.0;
            vector<double> log_intervals( maxima_t.size() - 1, 0 );
            for( size_t i = 1; i < maxima_t.size(); ++i ){
                double interval = maxima_t[i] - maxima_t[i-1];
                gm *= interval;
                log_intervals.push_back( log10(interval) );
            }
            GM_interv[n] = pow(gm, 1.0 / (maxima_t.size() - 1));
            SD_log[n] = gsl_stats_sd( log_intervals.data(), 1, log_intervals.size() );
            
            size_t mid_pos = (first_pos + last_pos) / 2;  // average: this rounds down
            mid_pos = start + ((mid_pos - start)/step)*step;    // must be in sync with step
            double pos_obs = 0.0, detect_lim = 10.0;
            for( size_t day = first_pos; day <= mid_pos; day += step ){
                if( dens[day] > detect_lim ) pos_obs += 1.0;
            }
            prop_pos_1st[n] = pos_obs / ((mid_pos - first_pos) / step + 1); // +1 because we count both first_pos and mid_pos
            pos_obs = 0.0;
            for( size_t day = mid_pos + step; day <= last_pos; day += step ){
                if( dens[day] > detect_lim ) pos_obs += 1.0;
            }
            prop_pos_2nd[n] = pos_obs / ((last_pos - mid_pos) / step);      // we don't count at mid_pos
        }
        
        /** This runs the infection model several times, capturing stats in the process */
        void capture(){
            const size_t N = init_slope.size();
            vector<double> dens;    // time series of density
            for( size_t run = 0; run < N; ++run ){
                dens.resize( 0 );
                MolineauxInfection* infection = new MolineauxInfection (0xFFFFFFFF);
                SimTime now = sim::ts0();
                
                while( !infection->update(1.0 /*no external immunity*/, now) ){
                    dens.push_back( infection->getDensity() );
                    now += sim::oneDay();
                }
                delete infection;
                calc( run, dens );
            }
            sort();
        }
        
        /** Sort stats: only do this after calculation of all stats. */
        void sort(){
            std::sort( init_slope.begin(), init_slope.end() );
            std::sort( log_1st_max.begin(), log_1st_max.end() );
            std::sort( no_max.begin(), no_max.end() );
            std::sort( slope_max.begin(), slope_max.end() );
            std::sort( GM_interv.begin(), GM_interv.end() );
            std::sort( SD_log.begin(), SD_log.end() );
            std::sort( prop_pos_1st.begin(), prop_pos_1st.end() );
            std::sort( prop_pos_2nd.begin(), prop_pos_2nd.end() );
            std::sort( last_pos_day.begin(), last_pos_day.end() );
        }
        
        /** Print stats (call sort() first) */
        void print(){
            // we look at median, 1st and 3rd quartiles, and 1st and 9th deciles
            // we also round to nearest, hence the addition part
            size_t last = init_slope.size() - 1;
            size_t med = (last + 1) / 2;
            size_t q1 = (last + 2) / 4, q3 = (last * 3 + 2) / 4;
            size_t c5 = (last + 10) / 20, c95 = (last * 19 + 10) / 20;
            
            std::cout << std::endl;
#define MOL_PRINT_STAT( s ) std::cout << "Stat " #s "\tc5: " << s[c5] << "\tq1: " << s[q1] << "\tmed: " << s[med] << "\tq3: " << s[q3] << "\tc95: " << s[c95] << std::endl;
            MOL_PRINT_STAT( init_slope );
            MOL_PRINT_STAT( log_1st_max );
            MOL_PRINT_STAT( no_max );
            MOL_PRINT_STAT( slope_max );
            MOL_PRINT_STAT( GM_interv );
            MOL_PRINT_STAT( SD_log );
            MOL_PRINT_STAT( prop_pos_1st );
            MOL_PRINT_STAT( prop_pos_2nd );
            MOL_PRINT_STAT( last_pos_day );
        }
        
        /** Write stats in a format we can read */
        void write( const char * file_name ){
            size_t last = init_slope.size() - 1;
            size_t med = (last + 1) / 2;
            size_t q1 = (last + 2) / 4, q3 = (last * 3 + 2) / 4;
            size_t c5 = (last + 10) / 20, c95 = (last * 19 + 10) / 20;
            
            std::ofstream file( file_name );
            file << std::setprecision( 5 );     // this is more precision than we need
            file << "stat\tc5\tq1\tmed\tq3\tc95" << std::endl;
#define MOL_WRITE_STAT( s ) file << #s << '\t' << s[c5] << '\t' << s[q1] << '\t' << s[med] << '\t' << s[q3] << '\t' << s[c95] << std::endl
            MOL_WRITE_STAT( init_slope );
            MOL_WRITE_STAT( log_1st_max );
            MOL_WRITE_STAT( no_max );
            MOL_WRITE_STAT( slope_max );
            MOL_WRITE_STAT( GM_interv );
            MOL_WRITE_STAT( SD_log );
            MOL_WRITE_STAT( prop_pos_1st );
            MOL_WRITE_STAT( prop_pos_2nd );
            MOL_WRITE_STAT( last_pos_day );
        }
        
        /** Read and compare stats */
        void compare( const char * file_name ){
            size_t last = init_slope.size() - 1;
            size_t med = (last + 1) / 2;
            size_t q1 = (last + 2) / 4, q3 = (last * 3 + 2) / 4;
            size_t c5 = (last + 10) / 20, c95 = (last * 19 + 10) / 20;
            
            std::ifstream file( file_name );
            TS_ASSERT( file );  // is the file open?
            if( !file ) return; // skip rest if file is not available
            
            std::string head;
            TS_ASSERT( file >> head && head == "stat" );
            TS_ASSERT( file >> head && head == "c5" );
            TS_ASSERT( file >> head && head == "q1" );
            TS_ASSERT( file >> head && head == "med" );
            TS_ASSERT( file >> head && head == "q3" );
            TS_ASSERT( file >> head && head == "c95" );
            
            double v_c5, v_q1, v_med, v_q3, v_c95;
            const double tol_rel = 1e-4, tol_abs = 1e-4;
#define MOL_CHECK_STAT( s ) \
            TS_ASSERT( file >> head >> v_c5 >> v_q1 >> v_med >> v_q3 >> v_c95 );\
            TS_ASSERT_EQUALS( head, #s );\
            TS_ASSERT_APPROX_TOL( v_c5, s[c5], tol_rel, tol_abs );\
            TS_ASSERT_APPROX_TOL( v_q1, s[q1], tol_rel, tol_abs );\
            TS_ASSERT_APPROX_TOL( v_med, s[med], tol_rel, tol_abs );\
            TS_ASSERT_APPROX_TOL( v_q3, s[q3], tol_rel, tol_abs );\
            TS_ASSERT_APPROX_TOL( v_c95, s[c95], tol_rel, tol_abs );
            MOL_CHECK_STAT( init_slope );
            MOL_CHECK_STAT( log_1st_max );
            MOL_CHECK_STAT( no_max );
            MOL_CHECK_STAT( slope_max );
            MOL_CHECK_STAT( GM_interv );
            MOL_CHECK_STAT( SD_log );
            MOL_CHECK_STAT( prop_pos_1st );
            MOL_CHECK_STAT( prop_pos_2nd );
            MOL_CHECK_STAT( last_pos_day );
        }
    };
};

#endif
