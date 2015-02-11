/* This file is part of OpenMalaria.
 * 
 * Copyright (C) 2005-2015 Swiss Tropical and Public Health Institute
 * Copyright (C) 2005-2015 Liverpool School Of Tropical Medicine
 * 
 * OpenMalaria is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * This file contains code internal to the LSTM PK/PD model to do with
 * dosing/scheduling.
 */
#ifndef Hmod_LSTMMedicate
#define Hmod_LSTMMedicate

#include "Global.h"
#include "PkPd/LSTMModel.h"
#include "util/errors.h"
#include "schema/pharmacology.h"

#include <vector>
#include <map>

using std::size_t;
using std::vector;
using std::map;

class UnittestUtil;
namespace OM { namespace PkPd {

struct Schedule {
    void load( const scnXml::PKPDSchedule::MedicateSequence& seq );
    
    vector<MedicateData> medications;
};

struct DosageTable {
    void load( const xsd::cxx::tree::sequence<scnXml::PKPDDosageRange>& seq, bool isBodyMass );
    
    /** Get dosage multilier from age or body mass.
     * 
     * Dosings may be given either by age or by body mass. It is assumed that
     * doses are given orally as some number of pills, thus multipliers will
     * usually be integers. IV doses, on the other hand, are given as mg/kg,
     * and this multilier will likely be one. */
    double getMultiplier( double key ){
        map<double,double>::const_iterator it = table.upper_bound( key );
        if( it == table.end() )
            throw TRACED_EXCEPTION( "bad age/dosage table", util::Error::PkPd );
        return it->second;
    }
    
    bool useMass;       // false: dosing by age; true: dosing by body mass
    map<double,double> table;
};

extern vector<Schedule> schedules;
extern vector<DosageTable> dosages;

} }
#endif
