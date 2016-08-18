typedef struct tagFRAME_DATA_PROP
{
	unsigned char L1_L2;
	unsigned char app_len;
	unsigned char app_cs;

	unsigned char rx_buf[128];
	unsigned char rx_idx;
	unsigned char rx_flag;
	unsigned char rx_time;

	unsigned char rx_frame_ok;

} FRAME_DATA_PROP, *PFRAME_DATA_PROP;
int main(void)
{

	FRAME_DATA_PROP p_data_prop;

memset(&p_data_prop,0,sizeof(p_data_prop));

return 0;
}
