/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "PHY/defs.h"

//#define NR_PSS_DEBUG

//short nr_mod_table[MOD_TABLE_SIZE_SHORT] = {0,0,768,768,-768,-768};

int nr_generate_pss(  int16_t *d_pss,
                      int32_t **txdataF,
                      int16_t amp,
                      uint8_t ssb_start_symbol,
                      nfapi_config_request_t* config,
                      NR_DL_FRAME_PARMS *frame_parms)
{
  int i,m,k,l;
  int16_t a, aa;
  int16_t x[NR_PSS_LENGTH];
  const int x_initial[7] = {0, 1, 1 , 0, 1, 1, 1};

  uint8_t Nid2 = config->sch_config.physical_cell_id.value % 3;

  /// Sequence generation
  for (i=0; i < 7; i++)
    x[i] = x_initial[i];

  for (i=0; i < (NR_PSS_LENGTH - 7); i++) {
    x[i+7] = (x[i + 4] + x[i]) %2;
  }

  for (i=0; i < NR_PSS_LENGTH; i++) {
    m = (i + 43*Nid2)%(NR_PSS_LENGTH);
    d_pss[i] = (1 - 2*x[m]) * 32767;
  }

#ifdef NR_PSS_DEBUG
  write_output("d_pss.m", "d_pss", (void*)d_pss, NR_PSS_LENGTH, 1, 1);
#endif

  /// Resource mapping
  a = (config->rf_config.tx_antenna_ports.value == 1) ? amp : (amp*ONE_OVER_SQRT2_Q15)>>15;

  for (aa = 0; aa < config->rf_config.tx_antenna_ports.value; aa++)
  {

    // PSS occupies a predefined position (subcarriers 56-182, symbol 0) within the SSB block starting from
    k = frame_parms->first_carrier_offset + frame_parms->ssb_start_subcarrier + 56; //and
    l = ssb_start_symbol;

    for (m = 0; m < NR_PSS_LENGTH; m++) {
      ((int16_t*)txdataF[aa])[2*(l*frame_parms->ofdm_symbol_size + k)] = (a * d_pss[m]) >> 15;
      k++;

      if (k >= frame_parms->ofdm_symbol_size)
        k-=frame_parms->ofdm_symbol_size;
    }
  }

#ifdef NR_PSS_DEBUG
  write_output("pss_0.m", "pss_0", (void*)txdataF[0][2*l*frame_parms->ofdm_symbol_size], frame_parms->ofdm_symbol_size, 1, 1);
#endif

  return (0);
}