#pragma once

#include "settingsIO.hpp"

/// Plain aggregate holding per-simulation mass parameters needed for
/// computing a mass-weighted centre of mass. Values are read from the
/// RunSetup file via SettingsIO.
struct MassParams
{
    double fraction_earth_mass_of_body1;
    double fraction_earth_mass_of_body2;
    double fraction_fe_body1;
    double fraction_si_body1;
    double fraction_fe_body2;
    double fraction_si_body2;
    double mass_of_earth;

    /// Factory: populate from an already-loaded SettingsIO instance.
    static MassParams fromSettingsIO(const SettingsIO& sio)
    {
        return {sio.getFractionEarthMassOfBody1(),
                sio.getFractionEarthMassOfBody2(),
                sio.getFractionFeBody1(),
                sio.getFractionSiBody1(),
                sio.getFractionFeBody2(),
                sio.getFractionSiBody2(),
                sio.getMassOfEarth()};
    }
};
