#pragma once

class RaceInfo {

    public :

        virtual ~RaceInfo() = 0;

        dword field04;
        dword field08;
        dword field0C;
        dword checkpoints_nb;
        dword laps_nb;

        dword field18;
        dword field1C;
        dword field20;
        dword status_time;
        dword status;
        dword field2C;

};