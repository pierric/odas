
   /**
    * \file     snk_samples.c
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

    #include <sink/snk_samples.h>

    snk_samples_obj * snk_samples_construct(const snk_samples_cfg * snk_samples_config) {

        snk_samples_obj * obj;
        uint size;

        obj = (snk_samples_obj *) malloc(sizeof(snk_samples_obj));

        obj->pots_min_energy = snk_samples_config->pots_min_energy;
        obj->nPots = snk_samples_config->nPots;
        memcpy(obj->hops_dims, snk_samples_config->hops_dims, sizeof(snk_samples_config->hops_dims));
        memcpy(obj->freqs_dims, snk_samples_config->freqs_dims, sizeof(snk_samples_config->freqs_dims));

        size = obj->hops_dims[0] * obj->hops_dims[1];
        obj->hops_buffer = (float *)malloc(sizeof(float) * size);
        size = obj->freqs_dims[0] * obj->freqs_dims[1];
        obj->freqs_buffer = (float *)malloc(sizeof(float) * size);
        obj->file = H5I_INVALID_HID;
        obj->hops_grp = H5I_INVALID_HID;
        obj->freqs_grp = H5I_INVALID_HID;

        //obj->format = format_clone(snk_samples_config->format);
        obj->interface = interface_clone(snk_samples_config->interface);

        if (!((obj->interface->type == interface_blackhole) ||
              (obj->interface->type == interface_file))) {

            printf("Sink pots: Invalid interface.\n");
            exit(EXIT_FAILURE);

        }

        obj->in_hops = (msg_hops_obj *) NULL;
        obj->in_pots = (msg_pots_obj *) NULL;
        obj->in_spectra = (msg_spectra_obj *) NULL;

        return obj;
    }

    void snk_samples_destroy(snk_samples_obj * obj) {

        //format_destroy(obj->format);
        interface_destroy(obj->interface);

        free(obj->hops_buffer);
        free(obj->freqs_buffer);

        free((void *) obj);

    }

    void snk_samples_connect(snk_samples_obj * obj, msg_hops_obj * in_hops, msg_pots_obj * in_pots, msg_spectra_obj * in_spectra) {

        obj->in_hops = in_hops;
        obj->in_pots = in_pots;
        obj->in_spectra = in_spectra;

    }

    void snk_samples_disconnect(snk_samples_obj * obj) {

        obj->in_hops = (msg_hops_obj *) NULL;
        obj->in_pots = (msg_pots_obj *) NULL;
        obj->in_spectra = (msg_spectra_obj *) NULL;

    }

    void snk_samples_open(snk_samples_obj * obj) {

        switch(obj->interface->type) {

            case interface_blackhole:

                snk_samples_open_interface_blackhole(obj);

            break;

            case interface_file:

                snk_samples_open_interface_file(obj);

            break;

            //case interface_socket:

            //    snk_samples_open_interface_socket(obj);

            //break;

            //case interface_terminal:

            //    snk_samples_open_interface_terminal(obj);

            //break;

            default:

                printf("Sink samples: Invalid interface type.\n");
                exit(EXIT_FAILURE);

            break;

        }

    }

    void snk_samples_open_interface_blackhole(snk_samples_obj * obj) {

        // Empty

    }

    void snk_samples_open_interface_file(snk_samples_obj * obj) {
        obj->file = H5Fcreate(obj->interface->fileName,
                              H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

        if (obj->file < 0) {
            printf("Cannot open file %s\n",obj->interface->fileName);
            exit(EXIT_FAILURE);
        }

        obj->hops_grp = H5Gcreate2(obj->file, "/hops", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        obj->freqs_grp = H5Gcreate2(obj->file, "/freqs", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        if (obj->hops_grp < 0 || obj->freqs_grp < 0) {
            printf("Cannot create groups\n");
            exit(EXIT_FAILURE);
        }
    }

    //void snk_samples_open_interface_socket(snk_samples_obj * obj) {

    //    memset(&(obj->sserver), 0x00, sizeof(struct sockaddr_in));

    //    obj->sserver.sin_family = AF_INET;
    //    obj->sserver.sin_addr.s_addr = inet_addr(obj->interface->ip);
    //    obj->sserver.sin_port = htons(obj->interface->port);
    //    obj->sid = socket(AF_INET, SOCK_STREAM, 0);

    //    if ( (connect(obj->sid, (struct sockaddr *) &(obj->sserver), sizeof(obj->sserver))) < 0 ) {

    //        printf("Sink samples: Cannot connect to server\n");
    //        exit(EXIT_FAILURE);

    //    }

    //}

    void snk_samples_close(snk_samples_obj * obj) {

        switch(obj->interface->type) {

            case interface_blackhole:

                snk_samples_close_interface_blackhole(obj);

            break;

            case interface_file:

                snk_samples_close_interface_file(obj);

            break;

            //case interface_socket:

            //    snk_samples_close_interface_socket(obj);

            //break;

            default:

                printf("Sink samples: Invalid interface type.\n");
                exit(EXIT_FAILURE);

            break;

        }

    }

    void snk_samples_close_interface_blackhole(snk_samples_obj * obj) {

        // Empty

    }

    void snk_samples_close_interface_file(snk_samples_obj * obj) {

        H5Gclose(obj->hops_grp);
        H5Gclose(obj->freqs_grp);
        H5Fclose(obj->file);

    }

    //void snk_samples_close_interface_socket(snk_samples_obj * obj) {

    //    close(obj->sid);

    //}

    static void record_array(const char *name, hid_t grp, uint indims[2], float *buffer, float *src[]) {
        hid_t dataspace, datatype, dataset;
        hsize_t dims[2];
        dims[0] = indims[0];
        dims[1] = indims[1];

        for (int r=0; r < dims[0]; r++) {
            memcpy(buffer + r * dims[1], src[r], sizeof(float) * dims[1]);
        }

        datatype = H5Tcopy(H5T_NATIVE_FLOAT);
        H5Tset_order(datatype, H5T_ORDER_LE);
        dataspace = H5Screate_simple(2, dims, NULL);
        dataset = H5Dcreate2(grp, name, datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        H5Dwrite(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);

        H5Sclose(dataspace);
        H5Tclose(datatype);
        H5Dclose(dataset);
    }

    int snk_samples_process(snk_samples_obj * obj) {

        int rtnValue;

        if (obj->in_pots->timeStamp != 0) {

            switch(obj->interface->type) {

                case interface_blackhole:

                    snk_samples_process_interface_blackhole(obj);

                break;

                case interface_file:

                    snk_samples_process_interface_file(obj);

                break;

                //case interface_socket:

                //    snk_samples_process_interface_socket(obj);

                //break;

                default:

                    printf("Sink samples: Invalid interface type.\n");
                    exit(EXIT_FAILURE);

                break;

            }

            rtnValue = 0;

        }
        else {

            rtnValue = -1;

        }

        return rtnValue;

    }

    void snk_samples_process_interface_blackhole(snk_samples_obj * obj) {

        // Empty

    }

    void snk_samples_process_interface_file(snk_samples_obj * obj) {

        float maxE = 0;
        char timestamp[16];

        snprintf(timestamp, 16, "%015d", obj->in_pots->timeStamp);

        for (int iPot = 0; iPot < obj->nPots; iPot++) {
            float e = obj->in_pots->pots->array[iPot * obj->nPots + 3];
            if (e > maxE)
                maxE = e;
        }
        if (maxE >= obj->pots_min_energy) {
            printf("Timestamp: %lld. Event! E=%f\n", obj->in_pots->timeStamp, maxE);

            record_array(timestamp, obj->hops_grp, obj->hops_dims, obj->hops_buffer, obj->in_hops->hops->array);
            record_array(timestamp, obj->freqs_grp, obj->freqs_dims, obj->freqs_buffer, obj->in_spectra->freqs->array);

        }

    }

    //void snk_samples_process_interface_socket(snk_samples_obj * obj) {

    //    if (send(obj->sid, obj->buffer, obj->bufferSize, 0) < 0) {
    //        printf("Sink samples: Could not send message.\n");
    //        exit(EXIT_FAILURE);
    //    }

    //}

    snk_samples_cfg * snk_samples_cfg_construct(void) {

        snk_samples_cfg * cfg;

        cfg = (snk_samples_cfg *) malloc(sizeof(snk_samples_cfg));

        cfg->pots_min_energy = 0;
        cfg->nPots = 0;
        memset(cfg->hops_dims, 0, sizeof(cfg->hops_dims));
        memset(cfg->freqs_dims, 0, sizeof(cfg->freqs_dims));
        //cfg->format = (format_obj *) NULL;
        cfg->interface = (interface_obj *) NULL;

        return cfg;

    }

    void snk_samples_cfg_destroy(snk_samples_cfg * snk_samples_config) {

        //if (snk_samples_config->format != NULL) {
        //    format_destroy(snk_samples_config->format);
        //}
        if (snk_samples_config->interface != NULL) {
            interface_destroy(snk_samples_config->interface);
        }

        free((void *) snk_samples_config);

    }
