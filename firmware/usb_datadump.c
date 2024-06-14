#include "usb_datadump.h"
#include <tusb.h>

uint32_t samp_freq;
bool mute;
uint16_t volume;
uint8_t clk_valid;
audio_control_range_4_n_t(1) samp_freq_range;
audio_control_range_2_n_t(1) vol_range;

void usb_datadump_task()
{

}

void usb_datadump_preinit()
{
	samp_freq = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
	clk_valid = 1;
	samp_freq_range.wNumSubRanges = 1;
	samp_freq_range.subrange[0].bMin = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
	samp_freq_range.subrange[1].bMin = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
	samp_freq_range.subrange[0].bRes = 0;
	vol_range.wNumSubRanges = 1;
	vol_range.subrange[0].bMin = -90;
	vol_range.subrange[0].bMax = 90;
	vol_range.subrange[0].bRes = 1;
}

bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const* p_request)
{
	uint8_t channel_num = TU_U16_LOW(p_request->wValue);
	uint8_t ctrl_sel = TU_U16_HIGH(p_request->wValue);
	uint8_t ent_id = TU_U16_HIGH(p_request->wIndex);

	if(ent_id == 1)
	{
		// The mic per-se entity
		if(ctrl_sel == AUDIO_TE_CTRL_CONNECTOR)
		{
			audio_desc_channel_cluster_t ret;
			ret.bNrChannels = 1;
			ret.bmChannelConfig = 0;
			ret.iChannelNames = 0;
			return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*)&ret, sizeof(ret));
		}
		else
			return false;
	}
	if(ent_id == 2)
	{
		// Feature unit
		if(ctrl_sel == AUDIO_FU_CTRL_MUTE)
		{
			return tud_control_xfer(rhport, p_request, &mute, 1);
		}
		if(ctrl_sel == AUDIO_FU_CTRL_VOLUME)
		{
			if(p_request->bRequest == AUDIO_CS_REQ_CUR)
			{
				return tud_control_xfer(rhport, p_request, &volume, sizeof(volume));
			}
			if(p_request->bRequest == AUDIO_CS_REQ_RANGE)
			{
				return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*)&vol_range, sizeof(vol_range));
			}

			return false;
		}
		return false;
	}
	if(ent_id == 4)
	{
		// Clock source
		if(ctrl_sel == AUDIO_CS_CTRL_SAM_FREQ)
		{
			if(p_request->bRequest == AUDIO_CS_REQ_CUR)
			{
				return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &samp_freq, sizeof(samp_freq));
			}
			if(p_request->bRequest == AUDIO_CS_REQ_RANGE)
			{
				return tud_control_xfer(rhport, p_request, &samp_freq_range, sizeof(samp_freq_range));
			}
			return false;
		}
		if(ctrl_sel == AUDIO_CS_CTRL_CLK_VALID)
		{
			return tud_control_xfer(rhport, p_request, &clk_valid, sizeof(clk_valid));
		}
		return false;
	}
	return false;
}
