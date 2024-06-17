#include "usb_datadump.h"
#include "adc.h"
#include <tusb.h>

uint32_t samp_freq;
bool mute;
uint16_t volume;
uint8_t clk_valid;
audio_control_range_4_n_t(1) samp_freq_range;
audio_control_range_2_n_t(1) vol_range;

bool datadump_task_block = false;

void usb_datadump_task()
{
	// This is critical: if one is already running, just ignore the repeated interrupt
	if(datadump_task_block)
	{
		return;
	}

	datadump_task_block = true;

	if(buffer_status == 1)
	{
		// First half ready
		tud_audio_write(adc_buffer, ADC_DATABUFFER / 2);
	}
	else if (buffer_status == 2)
	{
		// Second half ready
		tud_audio_write(adc_buffer + ADC_DATABUFFER / 2, ADC_DATABUFFER / 2);
	}

	// "Free" the block
	datadump_task_block = false;

	// Wait until next interrupt to keep processing
	buffer_status = 0;

}

void usb_datadump_preinit()
{
	samp_freq = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
	clk_valid = 1;
	samp_freq_range.wNumSubRanges = 1;
	samp_freq_range.subrange[0].bMin = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
	samp_freq_range.subrange[0].bMax = CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE;
	samp_freq_range.subrange[0].bRes = 0;
	vol_range.wNumSubRanges = 1;
	vol_range.subrange[0].bMin = -90;
	vol_range.subrange[0].bMax = 90;
	vol_range.subrange[0].bRes = 1;
}

bool tud_audio_set_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
	(void) rhport;
	(void) pBuff;

	// We do not support any set range requests here, only current value requests
	TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

	// Page 91 in UAC2 specification
	uint8_t channelNum = TU_U16_LOW(p_request->wValue);
	uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
	uint8_t ep = TU_U16_LOW(p_request->wIndex);

	(void) channelNum; (void) ctrlSel; (void) ep;

	return false; 	// Yet not implemented
}

// Invoked when audio class specific set request received for an interface
bool tud_audio_set_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
	(void) rhport;
	(void) pBuff;

	// We do not support any set range requests here, only current value requests
	TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

	// Page 91 in UAC2 specification
	uint8_t channelNum = TU_U16_LOW(p_request->wValue);
	uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
	uint8_t itf = TU_U16_LOW(p_request->wIndex);

	(void) channelNum; (void) ctrlSel; (void) itf;

	return false; 	// Yet not implemented
}

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
	(void) rhport;

	// Page 91 in UAC2 specification
	uint8_t channelNum = TU_U16_LOW(p_request->wValue);
	uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
	uint8_t itf = TU_U16_LOW(p_request->wIndex);
	uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

	(void) itf;

	// We do not support any set range requests here, only current value requests
	TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

	// If request is for our feature unit
	if ( entityID == 2 )
	{
		switch ( ctrlSel )
		{
			case AUDIO_FU_CTRL_MUTE:
				// Request uses format layout 1
				TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

				mute = ((audio_control_cur_1_t*) pBuff)->bCur;

			return true;

			case AUDIO_FU_CTRL_VOLUME:
				// Request uses format layout 2
				TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

				volume = (uint16_t) ((audio_control_cur_2_t*) pBuff)->bCur;

			return true;

				// Unknown/Unsupported control
			default:
				TU_BREAKPOINT();
			return false;
		}
	}
	return false;    // Yet not implemented
}

// Invoked when audio class specific get request received for an EP
bool tud_audio_get_req_ep_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
	(void) rhport;

	// Page 91 in UAC2 specification
	uint8_t channelNum = TU_U16_LOW(p_request->wValue);
	uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
	uint8_t ep = TU_U16_LOW(p_request->wIndex);

	(void) channelNum; (void) ctrlSel; (void) ep;

	//	return tud_control_xfer(rhport, p_request, &tmp, 1);

	return false; 	// Yet not implemented
}

// Invoked when audio class specific get request received for an interface
bool tud_audio_get_req_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
	(void) rhport;

	// Page 91 in UAC2 specification
	uint8_t channelNum = TU_U16_LOW(p_request->wValue);
	uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
	uint8_t itf = TU_U16_LOW(p_request->wIndex);

	(void) channelNum; (void) ctrlSel; (void) itf;

	return false; 	// Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
	(void) rhport;

	// Page 91 in UAC2 specification
	uint8_t channelNum = TU_U16_LOW(p_request->wValue);
	uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
	// uint8_t itf = TU_U16_LOW(p_request->wIndex); 			// Since we have only one audio function implemented, we do not need the itf value
	uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

	// Input terminal (Microphone input)
	if (entityID == 1)
	{
		switch ( ctrlSel )
		{
			case AUDIO_TE_CTRL_CONNECTOR:
			{
				// The terminal connector control only has a get request with only the CUR attribute.
				audio_desc_channel_cluster_t ret;

				// Those are dummy values for now
				ret.bNrChannels = 1;
				ret.bmChannelConfig = (audio_channel_config_t) 0;
				ret.iChannelNames = 0;


				return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &ret, sizeof(ret));
			}
			break;

				// Unknown/Unsupported control selector
			default:
				TU_BREAKPOINT();
				return false;
		}
	}

	// Feature unit
	if (entityID == 2)
	{
		switch ( ctrlSel )
		{
			case AUDIO_FU_CTRL_MUTE:
				// Audio control mute cur parameter block consists of only one byte - we thus can send it right away
				// There does not exist a range parameter block for mute
				return tud_control_xfer(rhport, p_request, &mute, 1);

			case AUDIO_FU_CTRL_VOLUME:
				switch ( p_request->bRequest )
				{
					case AUDIO_CS_REQ_CUR:
						return tud_control_xfer(rhport, p_request, &volume, sizeof(volume));

					case AUDIO_CS_REQ_RANGE:

						return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &vol_range, sizeof(vol_range));

						// Unknown/Unsupported control
					default:
						TU_BREAKPOINT();
						return false;
				}
			break;

				// Unknown/Unsupported control
			default:
				TU_BREAKPOINT();
				return false;
		}
	}

	// Clock Source unit
	if ( entityID == 4 )
	{
		switch ( ctrlSel )
		{
			case AUDIO_CS_CTRL_SAM_FREQ:
				// channelNum is always zero in this case
				switch ( p_request->bRequest )
				{
					case AUDIO_CS_REQ_CUR:
						// Buffered control transfer is needed for IN flow control to work
						return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &samp_freq, sizeof(samp_freq));

					case AUDIO_CS_REQ_RANGE:
						return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, &samp_freq_range, sizeof(samp_freq_range));

					 // Unknown/Unsupported control
					default:
						TU_BREAKPOINT();
						return false;
				}
			break;

			case AUDIO_CS_CTRL_CLK_VALID:
				// Only cur attribute exists for this request
				return tud_control_xfer(rhport, p_request, &clk_valid, sizeof(clk_valid));

			// Unknown/Unsupported control
			default:
				TU_BREAKPOINT();
				return false;
		}
	}

	return false; 	// Yet not implemented
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
	(void) rhport;
	(void) itf;
	(void) ep_in;
	(void) cur_alt_setting;


	// In read world application data flow is driven by I2S clock,
	// both tud_audio_tx_done_pre_load_cb() & tud_audio_tx_done_post_load_cb() are hardly used.
	// For example in your I2S receive callback:
	// void I2S_Rx_Callback(int channel, const void* data, uint16_t samples)
	// {
	//    tud_audio_write_support_ff(channel, data, samples * N_BYTES_PER_SAMPLE * N_CHANNEL_PER_FIFO);
	// }

	return true;
}

bool tud_audio_tx_done_post_load_cb(uint8_t rhport, uint16_t n_bytes_copied, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting)
{
	(void) rhport;
	(void) n_bytes_copied;
	(void) itf;
	(void) ep_in;
	(void) cur_alt_setting;

	return true;
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
	(void) rhport;
	(void) p_request;

	return true;
}


