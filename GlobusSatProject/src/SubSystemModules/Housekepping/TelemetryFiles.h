
#ifndef TELEMETRYFILES_H_
#define TELEMETRYFILES_H_
//	---general

#define END_FILE_NAME_TX_REVC			"txr"
#define END_FILE_NAME_TX				"tx"
#define END_FILE_NAME_RX				"rx"
#define END_FILE_NAME_RX_REVC           "rxr"
#define END_FILE_NAME_RX_FRAME 			"rxf"
#define END_FILE_NAME_ANTENNA			"ant"
#define END_FILENAME_WOD_TLM			"wod"
#define	END_FILENAME_EPS_RAW_MB_TLM		"erm"
#define END_FILENAME_EPS_ENG_MB_TLM		"eem"
#define END_FILENAME_EPS_RAW_CDB_TLM	"erc"
#define END_FILENAME_EPS_ENG_CDB_TLM	"eec"
#define	END_FILENAME_SOLAR_PANELS_TLM	"slr"
#define	END_FILENAME_LOGS				"log"



typedef enum {
	tlm_tx,
	tlm_tx_revc,
	tlm_rx,
	tlm_rx_revc,
	tlm_rx_frame,
	tlm_eps,
	tlm_wod,
	tlm_eps_raw_mb,
	tlm_eps_eng_mb,
	tlm_eps_raw_cdb,
	tlm_eps_eng_cdb,
	tlm_solar,
	tlm_antenna,
	tlm_log
}tlm_type_t;
#endif /* TELEMETRYFILES_H_ */
