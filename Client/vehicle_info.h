#pragma once

class VehicleInfo_ {

    public :

        dword fields_0[ 52 ];
        dword collidable;
        dword fields_1[ 75 ];
        dword field200;
        dword fields_2[ 40 ];

        float tyre_rot_x;
        float tyre_rot_y;
        float tyre_rot_z;

        float field2B0;
        float field2B4;
        float field2B8;
        float field2BC;

        float field2C0;

        float rot_x;
        float rot_y;
        float rot_z;

        float field2D0;

        float x;
        float y;
        float z;

        float field2E0;

        float velocity_x;
        float velocity_y;
        float velocity_z;
        

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