#pragma once

class VehicleInfo_ {

    public :

        dword hgw[ 128 ];
        dword field200;
        dword hgw2[ 52 ];
        float x;
        float y;
        float z;

};

class VehicleInfo {

    public :

        virtual ~VehicleInfo() = 0;

        dword field04;
        float field08;
        float field0C;
        float field10;
        float field14;

        float field18;
        float gear_ratios[ 7 ];
        dword gears_nb;
        float field3C;
        float field40;
        float field44;

        float field48;
        float field4C;
        float steering_coef;
        float field54;
        float field58;
        float field5C;

        float acceleration_strength;
        float handling_strength;
        dword current_gear;
        dword field6C;
        float steering_strength;
        dword field74;

        VehicleInfo_* info;

};