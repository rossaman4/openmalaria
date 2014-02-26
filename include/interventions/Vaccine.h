/* This file is part of OpenMalaria.
 * 
 * Copyright (C) 2005-2014 Swiss Tropical and Public Health Institute
 * Copyright (C) 2005-2014 Liverpool School Of Tropical Medicine
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
#ifndef Hmod_interventions_Vaccine
#define Hmod_interventions_Vaccine

#include "interventions/HumanComponents.h"
#include "schema/interventions.h"
#include <boost/shared_ptr.hpp>

namespace OM {
namespace Host {
    class Human;
}
namespace interventions {
    
/** Vaccine intervention parameters.
 *
 * Used to represent PEV, BSV and TBV vaccines.
 * Each that has a descriptor is applied
 * simultaneously by a continuous or timed intervention strategy (no
 * way to determine which are used).
 *
 * All parameters (inc. non-static) are only set by initParameters(). */
class VaccineEffect : public HumanInterventionEffect {
public:
    VaccineEffect( EffectId id, const scnXml::VaccineDescription& seq, Vaccine::Types type );
    
    void deploy( Host::Human& human, Deployment::Method method, VaccineLimits vaccLimits )const;
    
    virtual Effect::Type effectType() const;
    
#ifdef WITHOUT_BOINC
    virtual void print_details( std::ostream& out )const;
#endif
    
private:
    /** Get the initial efficacy of the vaccine.
     *
     * @param numPrevDoses The number of prior vaccinations of the individual. */
    double getInitialEfficacy (size_t numPrevDoses) const;

    inline static const VaccineEffect& getParams( EffectId effect ){
        assert( effect.id >= 0 && effect.id < params.size() && params[effect.id] != 0 );
        return *params[effect.id];
    }
    
    /// Vaccine effect type
    Vaccine::Types type;
    
    /// Function representing decay of effect
    boost::shared_ptr<util::DecayFunction> decayFunc;

    /* Vaccine type specific parameters
     * Initial mean efficacy, definition depends on vaccine type */
    vector<double> initialMeanEfficacy;
    // Distribution of efficacies among individuals, parameter to sample from beta dist.
    double efficacyB;

    /** @brief Vaccine static parameters
     * 
     * Each instance is either null or points to data for the vaccine effect
     * with the given effect ID.
     *
     * No memory management (only leak is at exit which OS deals with). */
    static vector<VaccineEffect*> params;
    
    //TODO(monitoring):
    /** Until the monitoring system is updated, only one type of vaccination
     * delivery can be reported. This is whichever is first configured. */
    static EffectId reportEffect;

    friend class PerHumanVaccine;
    friend class PerEffectPerHumanVaccine;
};

}
}
#endif
