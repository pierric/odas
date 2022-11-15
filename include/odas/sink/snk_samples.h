#ifndef __ODAS_SINK_SAMPLES
#define __ODAS_SINK_SAMPLES

   /**
    * \file     snk_samples.h
    * \author   Jiasen Wu <jiasenwu@hotmail.com>
    * \version  2.0
    * \date     2022-11-12
    * \copyright
    *
    * This program is free software: you can redistribute it and/or modify
    * it under the terms of the GNU General Public License as published by
    * the Free Software Foundation, either version 3 of the License, or
    * (at your option) any later version.
    *
    * This program is distributed in the hope that it will be useful,
    * but WITHOUT ANY WARRANTY; without even the implied warranty of
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    * GNU General Public License for more details.
    *
    * You should have received a copy of the GNU General Public License
    * along with this program.  If not, see <http://www.gnu.org/licenses/>.
    *
    */

    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <hdf5.h>

    #include "../general/format.h"
    #include "../general/interface.h"
    #include "../message/msg_hops.h"
    #include "../message/msg_pots.h"
    #include "../message/msg_spectra.h"

    typedef struct snk_samples_obj {

        float pots_min_energy;
        uint nPots;
        uint hops_dims[2];
        uint freqs_dims[2];
        float * hops_buffer;
        float * freqs_buffer;

        //format_obj * format;
        interface_obj * interface;

        msg_hops_obj    * in_hops;
        msg_pots_obj    * in_pots;
        msg_spectra_obj * in_spectra;

        hid_t file;
        hid_t hops_grp;
        hid_t freqs_grp;

    } snk_samples_obj;

    typedef struct snk_samples_cfg {

        float pots_min_energy;
        int nPots;
        uint hops_dims[2];
        uint freqs_dims[2];

        format_obj * format;
        interface_obj * interface;

    } snk_samples_cfg;

    snk_samples_obj * snk_samples_construct(const snk_samples_cfg * snk_samples_config);

    void snk_samples_destroy(snk_samples_obj * obj);

    void snk_samples_connect(snk_samples_obj * obj, msg_hops_obj * in_hops, msg_pots_obj * in_pots, msg_spectra_obj * in_spectra);

    void snk_samples_disconnect(snk_samples_obj * obj);

    void snk_samples_open(snk_samples_obj * obj);

    void snk_samples_open_interface_blackhole(snk_samples_obj * obj);

    void snk_samples_open_interface_file(snk_samples_obj * obj);

    //void snk_samples_open_interface_socket(snk_samples_obj * obj);

    void snk_samples_close(snk_samples_obj * obj);

    void snk_samples_close_interface_blackhole(snk_samples_obj * obj);

    void snk_samples_close_interface_file(snk_samples_obj * obj);

    //void snk_samples_close_interface_socket(snk_samples_obj * obj);

    int snk_samples_process(snk_samples_obj * obj);

    void snk_samples_process_interface_blackhole(snk_samples_obj * obj);

    void snk_samples_process_interface_file(snk_samples_obj * obj);

    void snk_samples_process_interface_socket(snk_samples_obj * obj);

    snk_samples_cfg * snk_samples_cfg_construct(void);

    void snk_samples_cfg_destroy(snk_samples_cfg * snk_samples_config);

#endif
