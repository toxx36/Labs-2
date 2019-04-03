/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Test for ZC application written using ZDO.
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"
#include "zb_secur.h"
#include "zb_secur_api.h"

#ifndef ZB_ED_ROLE
#error define ZB_ED_ROLE to compile ze tests
#endif
/*! \addtogroup ZB_TESTS */
/*! @{ */
#ifndef ZB_SECURITY
#error Define ZB_SECURITY
#endif

#define BULB_ADDRESS 0x0000
#define COMMAND_COUNT 1

/*! \addtogroup ZB_TESTS */
/*! @{ */


typedef struct ep_clusters_s
{
	zb_uint16_t profile;
	zb_uint16_t app_dev;
	zb_uint16_t app_ver;
	zb_uint8_t input_count;
	zb_uint16_t *input_list;
	zb_uint8_t output_count;
	zb_uint16_t *output_list;
} ep_clusters_t;

typedef struct ep_simple_desc_s
{
	zb_uint8_t ep_count;
	zb_uint8_t *ep_list;
	ep_clusters_t *clusters;	
} ep_simple_desc_t;

static ep_simple_desc_t *ep_desc = NULL;

static zb_uint8_t zr_remote_command;
//static zb_uint8_t new_buffer;
static zb_uint8_t cur_index = 0;


//static zb_uint8_t Command_send(zb_uint8_t command);

static void ieee_addr_req(zb_uint8_t param);
static void simple_desc_req(zb_uint8_t param);
static void active_ep_req(zb_uint8_t param);
static void power_desc_req(zb_uint8_t param);
static void send_to_clust(zb_uint8_t param);

static void simple_desc_output(zb_uint8_t offset);
static void ep_trace_output(void);

void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK;
void simple_desc_callback(zb_uint8_t param) ZB_CALLBACK;
void active_ep_callback(zb_uint8_t param) ZB_CALLBACK;
void node_power_desc_callback(zb_uint8_t param) ZB_CALLBACK;
void data_indication(zb_uint8_t param) ZB_CALLBACK;

void next_command(zb_uint8_t param) ZB_CALLBACK;
void send_command(zb_uint8_t param) ZB_CALLBACK;


zb_ieee_addr_t g_ieee_addr = {0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

MAIN()
{
	ARGV_UNUSED;

#ifndef KEIL
	if ( argc < 3 )
	{
		printf("%s <read pipe path> <write pipe path>\n", argv[0]);
		return 0;
	}
#endif

	/* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
	ZB_INIT("zdo_ze", argv[1], argv[2]);
#else
	ZB_INIT("zdo_ze", "2", "2");
#endif
	ZG->nwk.nib.security_level = 0;
	ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), g_ieee_addr);
	
	ep_desc = (ep_simple_desc_t*) realloc(ep_desc, sizeof(ep_simple_desc_t));
	ep_desc->ep_list = NULL;
	ep_desc->clusters = NULL;
	
	if (zdo_dev_start() != RET_OK)
	{
		TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
	}
	else
	{
		zdo_main_loop();
	}

	TRACE_DEINIT();
	MAIN_RETURN(0);
}


void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
	zb_buf_t *buf = ZB_BUF_FROM_REF(param);
	if (buf->u.hdr.status == 0)
	{
		TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
		zr_remote_command = 0;
		next_command(0);
	}
	else
	{
		TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
		zb_free_buf(buf);
	}
}

void next_command(zb_uint8_t param) ZB_CALLBACK ///Generate command and create new buffer
{
	ZVUNUSED(param);
	ZB_GET_OUT_BUF_DELAYED(send_command);
}

void send_command(zb_uint8_t param) ZB_CALLBACK ///Process the buffer and send command
{
	switch(zr_remote_command) {
		case 0:
			ieee_addr_req(param);
		break;
		case 1:
			power_desc_req(param);
	  break;
	 	case 2:
			active_ep_req(param);
		break;
		case 3:
			simple_desc_req(param);
		break;
		case 4:
			send_to_clust(param);
			zb_af_set_data_indication(data_indication);
		break;
		default:
		break;
	}
	TRACE_MSG(TRACE_APS2, "### end of switch", (FMT__0));	
}

void ieee_addr_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_zdo_nwk_addr_resp_head_t *resp;
  zb_ieee_addr_t ieee_addr;
  TRACE_MSG(TRACE_ZDO2, "zb_get_peer_short_addr_cb param %hd", (FMT__H, param));
  resp = (zb_zdo_nwk_addr_resp_head_t*)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_ZDO2, "#### resp status %hd, nwk addr %d", (FMT__H_D, resp->status, resp->nwk_addr));
  ZB_LETOH64(ieee_addr, resp->ieee_addr);
  ZB_DUMP_IEEE_ADDR(ieee_addr);
  ZB_SCHEDULE_CALLBACK(next_command, 0);
  zb_free_buf(buf);
}

void active_ep_callback(zb_uint8_t param) ZB_CALLBACK /* Save and trace endpoints list */
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_ep_resp_t *resp = (zb_zdo_ep_resp_t*)zdp_cmd;
  ep_desc->ep_count = resp->ep_count;
      TRACE_MSG(TRACE_APS1, "#####ep", (FMT__0));
  ep_desc->ep_list = (zb_uint8_t *)realloc(ep_desc->ep_list, sizeof(zb_uint8_t) * ep_desc->ep_count);
  memcpy(ep_desc->ep_list, zdp_cmd + sizeof(zb_zdo_ep_resp_t), sizeof(zb_uint8_t) * ep_desc->ep_count);
  
  TRACE_MSG(TRACE_ZDO2, "#### active_ep_callback status %hd, addr 0x%x",
            (FMT__H, resp->status, resp->nwk_addr));
  if (resp->status != ZB_ZDP_STATUS_SUCCESS || resp->nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }
  ep_trace_output();
  ZB_SCHEDULE_CALLBACK(next_command, 0);
  zb_free_buf(buf);
}

void simple_desc_callback(zb_uint8_t param) ZB_CALLBACK /* Process endpoint description */
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *output_count_pos;
  ep_clusters_t* cur_ep = ep_desc->clusters + cur_index;
  zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t*)ZB_BUF_BEGIN(buf);
  TRACE_MSG(TRACE_APS1, "simple_desc_callback status %hd, addr 0x%x",
            (FMT__H, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }
  else
  {
  	/* uint8 input_count - uint16 input_list[input_count] - uint8 output_count - uint16 output_list[output_count] */

  	cur_ep->input_count = resp->simple_desc.app_input_cluster_count;
  	output_count_pos = (zb_uint8_t*)(((zb_uint16_t *) (&resp->simple_desc.app_input_cluster_count + 1)) + resp->simple_desc.app_input_cluster_count); /* Move to start of input clusters - move to output clusters count */  
  	cur_ep->output_count = *output_count_pos;
  	
  	if(cur_ep->input_count > 0)
  	{
  		cur_ep->input_list = (zb_uint16_t*) calloc(cur_ep->input_count, sizeof(zb_uint16_t));
  		memcpy(cur_ep->input_list, &resp->simple_desc.app_input_cluster_count + 1, sizeof(zb_uint16_t) * cur_ep->input_count);
  	}
  	if(cur_ep->output_count > 0)
  	{
  		cur_ep->output_list = (zb_uint16_t*) calloc(cur_ep->output_count, sizeof(zb_uint16_t));
  		memcpy(cur_ep->output_list, output_count_pos + 1, sizeof(zb_uint16_t) * cur_ep->output_count);
  	}
    TRACE_MSG(TRACE_APS1, "### Clusters saved!", (FMT__0)); 
    
    cur_ep->app_ver = resp->simple_desc.app_device_version;
    cur_ep->app_dev = resp->simple_desc.app_device_id;
    cur_ep->profile = resp->simple_desc.app_profile_id;
    
		simple_desc_output(cur_index);
  }
	cur_index++;
	if(cur_index >= ep_desc->ep_count)
	{
		cur_index = 0;
		zr_remote_command++;
	}
  ZB_SCHEDULE_CALLBACK(next_command, 0);
  zb_free_buf(buf);
}


void data_indication(zb_uint8_t param) ZB_CALLBACK
{
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  /* Remove APS header from the packet */
  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS3, "###data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));
}

void node_power_desc_callback(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_uint8_t *zdp_cmd = ZB_BUF_BEGIN(buf);
  zb_zdo_power_desc_resp_t *resp = (zb_zdo_power_desc_resp_t*)(zdp_cmd);
  TRACE_MSG(TRACE_APS1, " node_power_desc_callback status %hd, addr 0x%x",
            (FMT__H, resp->hdr.status, resp->hdr.nwk_addr));
  if (resp->hdr.status != ZB_ZDP_STATUS_SUCCESS || resp->hdr.nwk_addr != 0x0)
  {
    TRACE_MSG(TRACE_APS1, "Error incorrect status/addr", (FMT__0));
  }
  TRACE_MSG(TRACE_APS1, "### power mode %hd, avail power src %hd, cur power src %hd, cur power level %hd",
            (FMT__H_H_H_H, ZB_GET_POWER_DESC_CUR_POWER_MODE(&resp->power_desc),
             ZB_GET_POWER_DESC_AVAIL_POWER_SOURCES(&resp->power_desc),
             ZB_GET_POWER_DESC_CUR_POWER_SOURCE(&resp->power_desc),
             ZB_GET_POWER_DESC_CUR_POWER_SOURCE_LEVEL(&resp->power_desc)));
  ZB_SCHEDULE_CALLBACK(next_command, 0);
  zb_free_buf(buf);
}

static void ep_trace_output(void)
{
	int i;
	TRACE_MSG(TRACE_APS1, "### Endpoint count: %d", (FMT__D, ep_desc->ep_count));
	if(ep_desc->ep_count > 0)
	{
		TRACE_MSG(TRACE_APS1, "Endpoint nums:", (FMT__0));
		for(i = 0; i < ep_desc->ep_count; i++)
		{
			TRACE_MSG(TRACE_APS1, "> Endpoint %d", (FMT__D, *(ep_desc->ep_list + i)));
		}
	}
}

static void simple_desc_output(zb_uint8_t offset)
{
	zb_uint_t i;
	TRACE_MSG(TRACE_APS1, "ep %hd, app prof 0x%04hx, dev id 0x%04hx, dev ver 0x%04hx, input clusters count %d, output clusters count %d",
							(FMT__H_D_D_H_H_H, *(ep_desc->ep_list+offset), (ep_desc->clusters+offset)->profile,
							(ep_desc->clusters+offset)->app_dev , (ep_desc->clusters+offset)->app_ver,
							(ep_desc->clusters+offset)->input_count, (ep_desc->clusters+offset)->output_count));
	           
	TRACE_MSG(TRACE_APS1, "### Input clusters:", (FMT__0));
	for(i = 0; i < ((ep_desc->clusters+offset)+offset)->input_count; i++)
	{
		TRACE_MSG(TRACE_APS1, "> 0x%04hx", (FMT__H, *((ep_desc->clusters+offset)->input_list + i)));
	}
	TRACE_MSG(TRACE_APS1, "### Output clusters:", (FMT__0));
	for(i = 0; i < (ep_desc->clusters+offset)->output_count; i++)
	{
		TRACE_MSG(TRACE_APS1, "> 0x%04hx", (FMT__H, *((ep_desc->clusters+offset)->output_list + i)));
	}
}

static void ieee_addr_req(zb_uint8_t param)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_zdo_ieee_addr_req_t *req_addr = NULL;
	ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_ieee_addr_req_t), req_addr);
	req_addr->nwk_addr = 0;
	req_addr->request_type = ZB_ZDO_SINGLE_DEV_RESPONSE;
	req_addr->start_index = 0;
	zb_zdo_ieee_addr_req(ZB_REF_FROM_BUF(buf), ieee_addr_callback);
	TRACE_MSG(TRACE_APS2, "### IEEE command sended", (FMT__0));
	zr_remote_command++;
}

static void simple_desc_req(zb_uint8_t param)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_zdo_simple_desc_req_t *req = NULL;

	ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_simple_desc_req_t), req);
	req->nwk_addr = 0; //send to coordinator
	req->endpoint = *(ep_desc->ep_list + cur_index);
	ep_desc->clusters = (ep_clusters_t*) realloc(ep_desc->clusters, sizeof(ep_clusters_t) * ep_desc->ep_count);	
	zb_zdo_simple_desc_req(ZB_REF_FROM_BUF(buf), simple_desc_callback);
	TRACE_MSG(TRACE_APS2, "### Simple desc command sended", (FMT__0));
}

static void active_ep_req(zb_uint8_t param)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_zdo_active_ep_req_t *req_ep = NULL;
	ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_active_ep_req_t), req_ep);
	req_ep->nwk_addr = 0; //coord addr
	zb_zdo_active_ep_req(ZB_REF_FROM_BUF(buf), active_ep_callback);
	TRACE_MSG(TRACE_APS2, "### EP command sended", (FMT__0));
	zr_remote_command++;
}

static void power_desc_req(zb_uint8_t param)
{
	zb_buf_t *buf = (zb_buf_t *)ZB_BUF_FROM_REF(param);
	zb_zdo_power_desc_req_t *req = NULL;
	ZB_BUF_INITIAL_ALLOC(buf, sizeof(zb_zdo_power_desc_req_t), req);
	req->nwk_addr = 0; //send to coordinator
	zb_zdo_power_desc_req(ZB_REF_FROM_BUF(buf), node_power_desc_callback);
	TRACE_MSG(TRACE_APS2, "### Power desc command sended", (FMT__0));
	zr_remote_command++;
}

static void send_to_clust(zb_uint8_t param)
{
	static zb_uint8_t cur_cluster = 0;
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req;
  zb_uint8_t *ptr = NULL;

	ep_clusters_t* cur_ep = ep_desc->clusters + cur_index; 
	
	if(cur_ep->input_count > 0)
	{

	  ZB_BUF_INITIAL_ALLOC(buf, 0, ptr);
	  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
	  req->dst_addr.addr_short = 0; /* send to ZC */
	  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
	  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
	  req->radius = 1;
		req->clusterid = *(cur_ep->input_list + cur_cluster); 
	  req->profileid = cur_ep->profile;
	  req->dst_endpoint = *(ep_desc->ep_list + cur_index);
	  req->src_endpoint = 10;
	  buf->u.hdr.handle = 0x11;

	  TRACE_MSG(TRACE_APS3, "###Sending apsde_data.request", (FMT__0));
		cur_cluster++;
	}
	
	if(cur_cluster >= cur_ep->input_count)
	{
		cur_cluster = 0;
		cur_index++;
		if(cur_index >= ep_desc->ep_count)
		{
			cur_index = 0;
			zr_remote_command++;
		}
	}
  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}


/*! @} */