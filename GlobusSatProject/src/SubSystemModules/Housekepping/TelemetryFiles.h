
#ifndef TELEMETRYFILES_H_
#define TELEMETRYFILES_H_
//	---general
#define FILENAME_WOD_TLM				"wod_tlm"

//	---eps
#define	FILENAME_EPS_RAW_MB_TLM			"eps_RawMB_tlm"
#define FILENAME_EPS_ENG_MB_TLM			"eps_EngMB_tlm"
#define FILENAME_EPS_RAW_CDB_TLM		"eps_RawCDB_tlm"
#define FILENAME_EPS_ENG_CDB_TLM		"eps_EngCDB_tlm"
#define	FILENAME_SOLAR_PANELS_TLM		"slrPnl_tlm"

//	---trxvu
#define FILENAME_TX_TLM					"tx_tlm"
#define FILENAME_TX_REVC				"tx_revc"
#define FILENAME_RX_TLM					"rx_tlm"
#define FILENAME_RX_REVC				"rx_revC"
#define FILENAME_RX_FRAME				"rx_frame"
#define FILENAME_ANTENNA_TLM			"ant_tlm"

#define TX_REVC_END_FILE_NAME			"txr"
#define TX_END_FILE_NAME				"tx"
#define RX_END_FILE_NAME				"rx"
#define RX_REVC_END_FILE_NAME           "rxr"
#define RX_FRAME_END_FILE_NAME 			"rxf"
#define ANTENNA_END_FILE_NAME			"ant"
typedef enum {
	tlm_tx,
	tlm_tx_revc,
	tlm_rx,
	tlm_rx_revc,
	tlm_rx_frame,

	tlm_wod,
	tlm_eps_raw_mb,
	tlm_eps_eng_mb,
	tlm_eps_raw_cdb,
	tlm_eps_eng_cdb,
	tlm_solar,
	tlm_antenna
}tlm_type_t;
#endif /* TELEMETRYFILES_H_ */
