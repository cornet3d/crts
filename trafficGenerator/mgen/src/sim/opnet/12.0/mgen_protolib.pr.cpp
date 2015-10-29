/* Process model C++ form file: mgen_protolib.pr.cpp */
/* Portions of this file copyright 1992-2006 by OPNET Technologies, Inc. */



/* This variable carries the header into the object file */
const char mgen_protolib_pr_cpp [] = "MIL_3_Tfile_Hdr_ 120A 30A op_runsim 7 465C41F6 465C41F6 1 wn12jh Jim@Hauser 0 0 none none 0 0 none 0 0 0 0 0 0 0 0 10de 3                                                                                                                                                                                                                                                                                                                                                                                                      ";
#include <string.h>



/* OPNET system definitions */
#include <opnet.h>



/* Header Block */

#include <OpnetMgenProcess.h>
#include <oms_pr.h>
#include <oms_tan.h>
#include <udp_api.h>
#include <ip_rte_v4.h>
#include <ip_addr_v4.h>
#include <mgenVersion.h>
#include <tcp_api_v3.h>
#include "animInfo.h"  // JPH - packet animation
#include <ip_rte_support.h>


/*	Define a transition conditions              	*/
#define	SELF_NOTIF		intrpt_type == OPC_INTRPT_SELF
#define	END_SIM			op_intrpt_type () == OPC_INTRPT_ENDSIM
#define	MSG_ARRIVAL		op_intrpt_type () == OPC_INTRPT_STRM
#define TIMEOUT_EVENT	((op_intrpt_type () == OPC_INTRPT_SELF) && (op_intrpt_code() == SELF_INTRT_CODE_TIMEOUT_EVENT))
#define	TCP_IND			op_intrpt_type () == OPC_INTRPT_REMOTE
#define RX_TO			((op_intrpt_type () == OPC_INTRPT_SELF) && (op_intrpt_code() == RX_ANIM_TO))
#define TX_TO			((op_intrpt_type () == OPC_INTRPT_SELF) && (op_intrpt_code() == TX_ANIM_TO))


/* Forward Declarations */
void config_transport ();
void config_norm ();
void get_host_addr ();

static const int MAX_SCRIPT = 128;
static const int MAX_LOG = 128;
static const int MAX_COMMAND = 256;
static const int MAXSTATFLOWS = 25;

static int tot_pkts_sent[MAXSTATFLOWS];
static int tot_pkts_rcvd[MAXSTATFLOWS];
static int tot_pkts_init = false;
static int tot_pkts_prnt = false;

/* JPH - packet animation */
static int animInit = OPC_FALSE;
int mgen_tcp_port[MAX_MGEN_PORTS];
int mgen_udp_port[MAX_MGEN_PORTS];
int mgen_tcp_port_cnt = 0;
int mgen_udp_port_cnt = 0;

/* End of Header Block */

#if !defined (VOSD_NO_FIN)
#undef	BIN
#undef	BOUT
#define	BIN		FIN_LOCAL_FIELD(_op_last_line_passed) = __LINE__ - _op_block_origin;
#define	BOUT	BIN
#define	BINIT	FIN_LOCAL_FIELD(_op_last_line_passed) = 0; _op_block_origin = __LINE__;
#else
#define	BINIT
#endif /* #if !defined (VOSD_NO_FIN) */



/* State variable definitions */
class mgen_protolib_state
	{
	private:
		/* Internal state tracking for FSM */
		FSM_SYS_STATE

	public:
		mgen_protolib_state (void);

		/* Destructor contains Termination Block */
		~mgen_protolib_state (void);

		/* State Variables */
		OpnetMgenProcess	       		mgen_proc                                       ;
		OpMgenSink*	            		mgen_sink                                       ;
		Objid	                  		my_id                                           ;
		Objid	                  		my_node_id                                      ;
		Objid	                  		my_pro_id                                       ;
		Objid	                  		my_udp_id                                       ;
		Objid	                  		my_tcp_id                                       ;
		IpT_Address	            		my_ip_addr                                      ;
		IpT_Address	            		my_ip_mask                                      ;
		Prohandle	              		own_prohandle                                   ;
		OmsT_Pr_Handle	         		own_process_record_handle                       ;
		char	                   		pid_string [512]                                ;
		char	                   		node_name [40]                                  ;
		ProtoTimerMgr	          		timer                                           ;
		ProtoSocket::Notifier*	 		udpnotifier                                     ;
		ProtoSocket::Notifier*	 		tcpnotifier                                     ;
		ProtoAddress	           		host_ipv4_addr                                  ;
		Stathandle	             		bits_rcvd_stathandle                            ;
		Stathandle	             		bitssec_rcvd_flow_stathandle[MAXSTATFLOWS]      ;
		Stathandle	             		bitssec_sent_flow_stathandle[MAXSTATFLOWS]      ;
		Stathandle	             		bitssec_rcvd_stathandle                         ;
		Stathandle	             		pkts_rcvd_stathandle                            ;
		Stathandle	             		pktssec_rcvd_flow_stathandle[MAXSTATFLOWS]      ;
		Stathandle	             		pktssec_sent_flow_stathandle[MAXSTATFLOWS]      ;
		Stathandle	             		pktssec_rcvd_stathandle                         ;
		Stathandle	             		bits_sent_stathandle                            ;
		Stathandle	             		bitssec_sent_stathandle                         ;
		Stathandle	             		pkts_sent_stathandle                            ;
		Stathandle	             		pktssec_sent_stathandle                         ;
		Stathandle	             		ete_delay_stathandle                            ;
		Stathandle	             		ete_delay_flow_stathandle[MAXSTATFLOWS]         ;
		Stathandle	             		bits_rcvd_gstathandle                           ;
		Stathandle	             		bitssec_rcvd_gstathandle                        ;
		Stathandle	             		pkts_rcvd_gstathandle                           ;
		Stathandle	             		pktssec_rcvd_gstathandle                        ;
		Stathandle	             		bits_sent_gstathandle                           ;
		Stathandle	             		bitssec_sent_gstathandle                        ;
		Stathandle	             		pkts_sent_gstathandle                           ;
		Stathandle	             		pktssec_sent_gstathandle                        ;
		Stathandle	             		ete_delay_gstathandle                           ;
		int	                    		udp_outstream_index                             ;
		int	                    		tcp_outstream_index                             ;
		int	                    		norm_outstream_index                            ;
		int	                    		local_port                                      ;
		FILE*	                  		script_fp                                       ;
		Ici*	                   		sink_ici                                        ;
		int	                    		pkts_sent[MAXSTATFLOWS]                         ;
		int	                    		pkts_rcvd[MAXSTATFLOWS]                         ;
		Andid	                  		tx_did                                          ;	/* JPH 11/14/06 - packet animation */
		Andid	                  		rx_did                                          ;	/* JPH 11/14/06 - packet animation */
		int	                    		my_pnum                                         ;	/* JPH 11/14/06 - packet animation */
		Evhandle	               		tx_timeout_ev                                   ;	/* JPH 11/14/06 - packet animation */
		Evhandle	               		rx_timeout_ev                                   ;	/* JPH 11/14/06 - packet animation */

		/* FSM code */
		void mgen_protolib (OP_SIM_CONTEXT_ARG_OPT);
		/* Diagnostic Block */
		void _op_mgen_protolib_diag (OP_SIM_CONTEXT_ARG_OPT);

#if defined (VOSD_NEW_BAD_ALLOC)
		void * operator new (size_t) throw (VOSD_BAD_ALLOC);
#else
		void * operator new (size_t);
#endif
		void operator delete (void *);

		/* Memory management */
		static VosT_Obtype obtype;
	};

VosT_Obtype mgen_protolib_state::obtype = (VosT_Obtype)OPC_NIL;

#define mgen_proc               		op_sv_ptr->mgen_proc
#define mgen_sink               		op_sv_ptr->mgen_sink
#define my_id                   		op_sv_ptr->my_id
#define my_node_id              		op_sv_ptr->my_node_id
#define my_pro_id               		op_sv_ptr->my_pro_id
#define my_udp_id               		op_sv_ptr->my_udp_id
#define my_tcp_id               		op_sv_ptr->my_tcp_id
#define my_ip_addr              		op_sv_ptr->my_ip_addr
#define my_ip_mask              		op_sv_ptr->my_ip_mask
#define own_prohandle           		op_sv_ptr->own_prohandle
#define own_process_record_handle		op_sv_ptr->own_process_record_handle
#define pid_string              		op_sv_ptr->pid_string
#define node_name               		op_sv_ptr->node_name
#define timer                   		op_sv_ptr->timer
#define udpnotifier             		op_sv_ptr->udpnotifier
#define tcpnotifier             		op_sv_ptr->tcpnotifier
#define host_ipv4_addr          		op_sv_ptr->host_ipv4_addr
#define bits_rcvd_stathandle    		op_sv_ptr->bits_rcvd_stathandle
#define bitssec_rcvd_flow_stathandle		op_sv_ptr->bitssec_rcvd_flow_stathandle
#define bitssec_sent_flow_stathandle		op_sv_ptr->bitssec_sent_flow_stathandle
#define bitssec_rcvd_stathandle 		op_sv_ptr->bitssec_rcvd_stathandle
#define pkts_rcvd_stathandle    		op_sv_ptr->pkts_rcvd_stathandle
#define pktssec_rcvd_flow_stathandle		op_sv_ptr->pktssec_rcvd_flow_stathandle
#define pktssec_sent_flow_stathandle		op_sv_ptr->pktssec_sent_flow_stathandle
#define pktssec_rcvd_stathandle 		op_sv_ptr->pktssec_rcvd_stathandle
#define bits_sent_stathandle    		op_sv_ptr->bits_sent_stathandle
#define bitssec_sent_stathandle 		op_sv_ptr->bitssec_sent_stathandle
#define pkts_sent_stathandle    		op_sv_ptr->pkts_sent_stathandle
#define pktssec_sent_stathandle 		op_sv_ptr->pktssec_sent_stathandle
#define ete_delay_stathandle    		op_sv_ptr->ete_delay_stathandle
#define ete_delay_flow_stathandle		op_sv_ptr->ete_delay_flow_stathandle
#define bits_rcvd_gstathandle   		op_sv_ptr->bits_rcvd_gstathandle
#define bitssec_rcvd_gstathandle		op_sv_ptr->bitssec_rcvd_gstathandle
#define pkts_rcvd_gstathandle   		op_sv_ptr->pkts_rcvd_gstathandle
#define pktssec_rcvd_gstathandle		op_sv_ptr->pktssec_rcvd_gstathandle
#define bits_sent_gstathandle   		op_sv_ptr->bits_sent_gstathandle
#define bitssec_sent_gstathandle		op_sv_ptr->bitssec_sent_gstathandle
#define pkts_sent_gstathandle   		op_sv_ptr->pkts_sent_gstathandle
#define pktssec_sent_gstathandle		op_sv_ptr->pktssec_sent_gstathandle
#define ete_delay_gstathandle   		op_sv_ptr->ete_delay_gstathandle
#define udp_outstream_index     		op_sv_ptr->udp_outstream_index
#define tcp_outstream_index     		op_sv_ptr->tcp_outstream_index
#define norm_outstream_index    		op_sv_ptr->norm_outstream_index
#define local_port              		op_sv_ptr->local_port
#define script_fp               		op_sv_ptr->script_fp
#define sink_ici                		op_sv_ptr->sink_ici
#define pkts_sent               		op_sv_ptr->pkts_sent
#define pkts_rcvd               		op_sv_ptr->pkts_rcvd
#define tx_did                  		op_sv_ptr->tx_did
#define rx_did                  		op_sv_ptr->rx_did
#define my_pnum                 		op_sv_ptr->my_pnum
#define tx_timeout_ev           		op_sv_ptr->tx_timeout_ev
#define rx_timeout_ev           		op_sv_ptr->rx_timeout_ev

/* These macro definitions will define a local variable called	*/
/* "op_sv_ptr" in each function containing a FIN statement.	*/
/* This variable points to the state variable data structure,	*/
/* and can be used from a C debugger to display their values.	*/
#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE
#define FIN_PREAMBLE_DEC	mgen_protolib_state *op_sv_ptr;
#define FIN_PREAMBLE_CODE	\
		op_sv_ptr = ((mgen_protolib_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr));


/* Function Block */

#if !defined (VOSD_NO_FIN)
enum { _op_block_origin = __LINE__ + 2};
#endif

/* Initialization function */

void mgen_init ()
	{
	FIN (mgen_init ())
	config_transport ();

	/* Initilaize the statistic handles to keep	*/
	/* track of traffic sent and sinked by this process.	*/
	bits_rcvd_stathandle 		= op_stat_reg ("MGEN.Traffic Received (bits)",			OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	bitssec_rcvd_stathandle 	= op_stat_reg ("MGEN.Traffic Received (bits/sec)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	pkts_rcvd_stathandle 		= op_stat_reg ("MGEN.Traffic Received (packets)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	pktssec_rcvd_stathandle 	= op_stat_reg ("MGEN.Traffic Received (packets/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	bits_sent_stathandle 		= op_stat_reg ("MGEN.Traffic Sent (bits)",			OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	bitssec_sent_stathandle 	= op_stat_reg ("MGEN.Traffic Sent (bits/sec)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	pkts_sent_stathandle 		= op_stat_reg ("MGEN.Traffic Sent (packets)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	pktssec_sent_stathandle 	= op_stat_reg ("MGEN.Traffic Sent (packets/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
	ete_delay_stathandle		= op_stat_reg ("MGEN.End-to-End Delay (seconds)",		OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);

	bits_rcvd_gstathandle 		= op_stat_reg ("MGEN.Traffic Received (bits)",			OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	bitssec_rcvd_gstathandle 	= op_stat_reg ("MGEN.Traffic Received (bits/sec)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	pkts_rcvd_gstathandle 		= op_stat_reg ("MGEN.Traffic Received (packets)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	pktssec_rcvd_gstathandle 	= op_stat_reg ("MGEN.Traffic Received (packets/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	bits_sent_gstathandle 		= op_stat_reg ("MGEN.Traffic Sent (bits)",			OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	bitssec_sent_gstathandle 	= op_stat_reg ("MGEN.Traffic Sent (bits/sec)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	pkts_sent_gstathandle 		= op_stat_reg ("MGEN.Traffic Sent (packets)",		OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	pktssec_sent_gstathandle 	= op_stat_reg ("MGEN.Traffic Sent (packets/sec)",	OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	ete_delay_gstathandle		= op_stat_reg ("MGEN.End-to-End Delay (seconds)",	OPC_STAT_INDEX_NONE, OPC_STAT_GLOBAL);
	
	for (int i = 0; i<MAXSTATFLOWS; i++)
		{
		char s[50];
		sprintf(s,"MGEN.End-to-End Delay flow %i (seconds)",i);
		ete_delay_flow_stathandle[i] = op_stat_reg (s, OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
		sprintf(s,"MGEN.Traffic Received flow %i (bits/sec)",i);
		bitssec_rcvd_flow_stathandle[i] = op_stat_reg (s, OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
		sprintf(s,"MGEN.Traffic Received flow %i (pkts/sec)",i);
		pktssec_rcvd_flow_stathandle[i] = op_stat_reg (s, OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
		sprintf(s,"MGEN.Traffic Sent flow %i (bits/sec)",i);
		bitssec_sent_flow_stathandle[i] = op_stat_reg (s, OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
		sprintf(s,"MGEN.Traffic Sent flow %i (pkts/sec)",i);
		pktssec_sent_flow_stathandle[i] = op_stat_reg (s, OPC_STAT_INDEX_NONE, OPC_STAT_LOCAL);
		pkts_sent[i] = 0;  // pkts/flow/node
		pkts_rcvd[i] = 0;  // pkts/flow/node
		}
	
	if (!tot_pkts_init)
		{
		tot_pkts_init = true;
		for (int i = 0; i<MAXSTATFLOWS; i++)
			{
			tot_pkts_sent[i] = 0; // pkts/flow for all nodes
			tot_pkts_rcvd[i] = 0; // pkts/flow for all nodes
			}
		}
	
	tx_did = -1;  // JPH 11/14/06 - packet animation
	rx_did = -1;  // JPH 11/14/06 - packet animation

	FOUT;
	}


void stop()
	{
	FIN(stop());
	mgen_proc.OnShutdown();
	if (!tot_pkts_prnt)
		{
		tot_pkts_prnt = true;
		FILE* statfile;
		char sfname[256];
		char mfname[256];
		char fname[512]; 
		if (op_ima_sim_attr_exists("Stats File"))
			op_ima_sim_attr_get(OPC_IMA_STRING,"Stats File", sfname);
		else
			printf("MGEN Warning: Stats File sim attribute does not exist.\n");
		if (op_ima_sim_attr_exists("Motion File Name"))
			op_ima_sim_attr_get(OPC_IMA_STRING,"Motion File Name", mfname);
		else
			printf("MGEN Warning: Motion File Name sim attribute does not exist.\n");
		sprintf(fname,"%s%s",sfname,mfname);
		statfile = fopen(fname,"w");
		printf("\nMGEN SUMMARY STATS:\n");  
		for (int i = 0; i<MAXSTATFLOWS; i++)
			{
			if (tot_pkts_sent[i]+tot_pkts_rcvd[i] > 0)
				{
				double ratio = (double)tot_pkts_rcvd[i]/(double)tot_pkts_sent[i];
				printf("  Flow %d  tot_pkts_sent = %d  tot_pkts_rcvd = %d  sent/rcvd = %f\n",
					i, tot_pkts_sent[i], tot_pkts_rcvd[i], ratio);
				if (statfile)
					fprintf(statfile,"Flow%d  %d/%d  percent %f\n",
					i, tot_pkts_rcvd[i], tot_pkts_sent[i], ratio);
				}
			}
		printf("\nStats written to:\n  %s\n", fname);
		if (statfile)
			fclose(statfile);
		}

	FOUT;
	}


void
mgen_fatal_error (char *emsg)
	{
	char info[40];
	
	/** Abort the simulation with the given message. **/
	FIN (mgen_fatal_error (emsg));
   
	sprintf(info, "MGEN Error(%s):", node_name);
	op_sim_end (info, emsg, OPC_NIL, OPC_NIL);

	FOUT;
	}


void
mgen_warn_error (char *wmsg)
	{
	char info[40];
	
	/** Issue an warning (used for potentially inconsistent situations). **/
	FIN (mgen_warn_error (wmsg));
	
	sprintf(info, "MGEN Warning(%s):", node_name);

	if (op_prg_odb_ltrace_active ("mgen warn"))
		op_prg_odb_print_minor (info, wmsg, OPC_NIL);

	FOUT;
	}



/*  Function to configure the proper input/output streams to the udp process 
    and setup the udp port numbers via the ICI facility  */
void
config_transport()
	{
	int					outstrm_count;
	Objid				outstrm_objid;
    List*			 	proc_record_handle_list_ptr;
    OmsT_Pr_Handle		temp_process_record_handle;

	FIN(config_transport());
	
	/*                                                         */
	/* Get the outgoing packet stream index to the UDP process */
	/*                                                         */
	
    /* First, get the number of output streams for the MGEN process. */
    outstrm_count = op_topo_assoc_count (my_id, OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_STRM);
    if (outstrm_count < 2)
    	mgen_fatal_error ("MGEN does not have at least two outgoing streams - one to TCP, one to UDP");
	/* Then, get the outgoing stream Objid */
    outstrm_objid = op_topo_assoc (my_id, OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_STRM, 0);
	/* Retrieve the index of the stream. */
	op_ima_obj_attr_get (outstrm_objid, "src stream", &udp_outstream_index);
    outstrm_objid = op_topo_assoc (my_id, OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_STRM, 1);
	/* Retrieve the index of the stream. */
	op_ima_obj_attr_get (outstrm_objid, "src stream", &tcp_outstream_index);
	if (outstrm_count == 3)
		{
		outstrm_objid = op_topo_assoc (my_id, OPC_TOPO_ASSOC_OUT, OPC_OBJTYPE_STRM, 2);
		/* Retrieve the index of the stream. */
		op_ima_obj_attr_get (outstrm_objid, "src stream", &norm_outstream_index);
		}

    /* 						                                                    */
    /* Locate the UDP module. It must have registered in the process registry.  */
    /* 						                                                    */
	
    /* Obtain the process handles by matching the specific descriptors			*/
    proc_record_handle_list_ptr = op_prg_list_create ();
    oms_pr_process_discover (OPC_OBJID_INVALID, proc_record_handle_list_ptr,
    	"node objid",	OMSC_PR_OBJID,		my_node_id,
    	"protocol",		OMSC_PR_STRING,		"udp",
    	OPC_NIL);

    /* Each node can have only one UDP module.	*/
    if (op_prg_list_size (proc_record_handle_list_ptr) != 1)
    	{
    	/* Having more than one UDP module is a serious error.  End simulation. */
	    mgen_fatal_error ("Error: either zero or several UDP processes found in the local node"); 
    	}
    temp_process_record_handle = (OmsT_Pr_Handle) op_prg_list_access (proc_record_handle_list_ptr, OPC_LISTPOS_TAIL);

    /* Obtain the module objid of the udp module	*/
    oms_pr_attr_get (temp_process_record_handle, "module objid",OMSC_PR_OBJID, &my_udp_id);
	
	mgen_proc.SetUdpProcessId(my_udp_id);  /*  - OpnetProtoSimProcess.h */

    /* Deallocate the memory allocated for holding the record handle	*/
    while (op_prg_list_size (proc_record_handle_list_ptr) > 0)
    	op_prg_list_remove (proc_record_handle_list_ptr, OPC_LISTPOS_HEAD);

    /* Deallocate the temporary list pointer. */
    op_prg_mem_free (proc_record_handle_list_ptr);
	
	/* Register mgen with tcp api */
	ApiT_Tcp_App_Handle hndl = tcp_app_register(my_id);
	mgen_proc.SetTcpHandle(hndl);
	
	FOUT;
	}


/* The following function is used to obtain the IP unicast address
   for the subnet connected to the interface that MGEN is using.
*/
void get_host_addr ()
	{
	FIN (get_host_addr());

	
	Boolean gateway_status;
	char addr_str[20], mask_str[20];
	Objid ip_params_objid = ip_rte_parameters_objid_obtain (my_node_id, OPC_OBJID_NULL, &gateway_status);
	Objid iface_info_objid;
	if (op_ima_obj_attr_get (ip_params_objid, "Interface Information", &iface_info_objid) == OPC_COMPCODE_FAILURE)
		op_sim_end("Error: failed to get Interface Information objid", "", "", "");
	Objid intf_info_row_objid = op_topo_child (iface_info_objid, OPC_OBJTYPE_GENERIC,0);
	if (op_ima_obj_attr_get (intf_info_row_objid, "Address", addr_str) == OPC_COMPCODE_FAILURE)
		op_sim_end("Error: failed to get interface ip address", "", "", "");
	if (op_ima_obj_attr_get (intf_info_row_objid, "Subnet Mask", mask_str) == OPC_COMPCODE_FAILURE)
		op_sim_end("Error: failed to get interface ip subnet mask", "", "", "");
	my_ip_addr = ip_address_create(addr_str);
	my_ip_mask = ip_address_create(mask_str);
	host_ipv4_addr.SimSetAddress(my_ip_addr);
	mgen_proc.SetHostAddress(host_ipv4_addr);
	mgen_proc.SetTcpHostAddress(host_ipv4_addr);

	FOUT;
	}




OpnetMgenProcess::OpnetMgenProcess()
	: mgen(GetTimerMgr(), GetSocketNotifier())
	{
	}


OpnetMgenProcess::~OpnetMgenProcess()
	{
	
	}


const char* const OpnetMgenProcess::CMD_LIST[] =
{
    "+port",
    "-ipv6",       // open IPv6 sockets by default
    "-ipv4",       // open IPv4 sockets by default
    "+convert",    // convert binary logfile to text-based logfile
    "+sink",       // set Mgen::sink to stream sink
    "-block",      // set Mgen::sink to blocking I/O
    "+source",     // specify an MGEN stream source
    "+ifinfo",     // get tx/rx frame counts for specified interface
    "+precise",    // turn on/off precision packet timing
    "+instance",   // indicate mgen instance name 
    "-stop",       // exit program instance
    "+command",    // specifies an input command file/device
    "+hostAddr",   // turn "host" field on/off in sent messages
    "-help",       // print usage and exit
    NULL
};


OpnetMgenProcess::CmdType OpnetMgenProcess::GetCmdType(const char* cmd)
{
	FIN(GetCmdType(cmd));
    if (!cmd) FRET (CMD_INVALID);
    unsigned int len = strlen(cmd);
    bool matched = false;
    CmdType type = CMD_INVALID;
    const char* const* nextCmd = CMD_LIST;
    while (*nextCmd)
    {
        if (!strncmp(cmd, *nextCmd+1, len))
        {
            if (matched)
            {
                // ambiguous command (command should match only once)
                FRET (CMD_INVALID);
            }
            else
            {
                matched = true;   
                if ('+' == *nextCmd[0])
                    type = CMD_ARG;
                else
                    type = CMD_NOARG;
            }
        }
        nextCmd++;
    }
    FRET (type); 
}  // end OpnetMgenProcess::GetCmdType()


bool OpnetMgenProcess::ProcessCommands(int argc, const char*const* argv)
{
	FIN (ProcessCommands(argc,argv));
    // Group command line arguments into MgenApp or Mgen commands
    // and their (if applicable) respective arguments
    // Parse command line
    int i = 1;
    convert = false;  // initialize conversion flag
    while (i < argc)
    {
        CmdType cmdType = GetCmdType(argv[i]);
        if (CMD_INVALID == cmdType)
        {
            // Is it a class MgenApp command?
            switch(Mgen::GetCmdType(argv[i]))
            {
                case Mgen::CMD_INVALID:
                    break;
                case Mgen::CMD_NOARG:
                    cmdType = CMD_NOARG;
                    break;
                case Mgen::CMD_ARG:
                    cmdType = CMD_ARG;
                    break;
            }
        }
        switch (cmdType)
        {
            case CMD_INVALID:
                DMSG(0, "OpnetMgenProcess::ProcessCommands() error: invalid command:%s\n", argv[i]);
                return false;
            case CMD_NOARG:
                if (!OnCommand(argv[i], NULL))
                {
                    DMSG(0, "OpnetMgenProcess::ProcessCommands() OnCommand(%s) error\n", 
                            argv[i]);
                    FRET (false);
                }
                i++;
                break;
            case CMD_ARG:
                if (!OnCommand(argv[i], argv[i+1]))
                {
                    DMSG(0, "OpnetMgenProcess::ProcessCommands() OnCommand(%s, %s) error\n", 
                            argv[i], argv[i+1]);
                    FRET (false);
                }
                i += 2;
                break;
        }
    }  
    FRET (true);
}  // end OpnetMgenProcess::ProcessCommands()



bool OpnetMgenProcess::OnCommand(const char* cmd, const char* val)
{
	FIN (OnCommand(cmd,val));
#ifndef OPNET  // JPH - remote commands not currently implemented for Opnet
    if (control_remote)
    {
        char buffer[8192];
        strcpy(buffer, cmd);
        if (val)
        {
            strcat(buffer, " ");
            strncat(buffer, val, 8192 - strlen(buffer));
        }
        unsigned int len = strlen(buffer);
        if (control_pipe.Send(buffer, len))
        {
            return true;
        }
        else
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(%s) error sending command to remote process\n", cmd);    
            return false;
        }        
    }
#endif //OPNET
    CmdType type = GetCmdType(cmd);
    unsigned int len = strlen(cmd);    
    if (CMD_INVALID == type)
    {
        // If it is a core mgen command, process it as "overriding"
        if (Mgen::CMD_INVALID != Mgen::GetCmdType(cmd))
        {
            FRET (mgen.OnCommand(Mgen::GetCommandFromString(cmd), val, true));   
        }
        else
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(%s) error: invalid command\n", cmd);
            FRET (false);
        }
    }
    else if ((CMD_ARG == type) && !val)
    {
        DMSG(0, "OpnetMgenProcess::ProcessCommand(%s) missing argument\n", cmd);
        FRET (false);
    }
    else if (!strncmp("port", cmd, len))
    {
        // "port" == implicit "0.0 LISTEN UDP <val>" script
        char* string = new char[strlen(val) + 64];
        if (!string)
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(port) memory allocation error: %s\n",
                    GetErrorString());
            FRET (false);   
        }
        sprintf(string, "0.0 LISTEN UDP %s", val);
        DrecEvent* event = new DrecEvent;
        if (!event)
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(port) memory allocation error: %s\n",
                    GetErrorString());
            FRET (false);   
        }
        if (!event->InitFromString(string))
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(port) error parsing <portList>\n");
            FRET (false);      
        }
        have_ports = true;
        mgen.InsertDrecEvent(event);
    }
    else if (!strncmp("ipv4", cmd, len))
    {
        mgen.SetDefaultSocketType(ProtoAddress::IPv4);
    }
    else if (!strncmp("ipv6", cmd, len))
    {
#ifdef HAVE_IPV6 
        ProtoSocket::SetHostIPv6Capable();
        if (ProtoSocket::HostIsIPv6Capable())
            mgen.SetDefaultSocketType(ProtoAddress::IPv6);
        else
#endif // HAVE_IPV6
            DMSG(0, "OpnetMgenProcess::ProcessCommand(ipv6) Warning: system not IPv6 capable?\n");
    }
    else if (!strncmp("background", cmd, len))
    {
        // do nothing (this command was scanned immediately at startup)
    }
    else if (!strcmp("convert", cmd))
    {
        convert = true;             // set flag to do the conversion
        strcpy(convert_path, val);  // save path of file to convert
    }

    else if (!strncmp("sink", cmd, len))
    {
        mgen.SetSinkPath(val);
    }
    else if (!strncmp("block", cmd, len))
    {
        sink_non_blocking = false;
    }
	
#ifndef OPNET // dispatcher not implemented for Opnet
    else if (!strncmp("source", cmd, len))
    {
        MgenStreamSource* theSource = new MgenStreamSource(mgen, *this);
        if (theSource)
        {
            if (theSource->Open(val))
            {
                if (!dispatcher.InstallGenericInput(theSource->GetDescriptor(),
                                                    MgenStreamSource::DoInputReady, 
                                                    theSource))
                {
                    DMSG(0, "OpnetMgenProcess::ProcessCommand(source) Error: couldn't install stream source\n");
                    theSource->Close();
                    return false;       
                }  
                if (source) 
                {
                    source->Close();
                    delete source;  
                }             
                source = theSource;
            }
            else
            {
                DMSG(0, "MgenApp::ProcessCommand(source) Error: couldn't open stream source\n");
                return false;
            }                           
        }
        else
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(source) Error: couldn't allocate source: %s\n",
                    GetErrorString());
            return false;
        }
    }
    else if (!strncmp("ifinfo", cmd, len))
    {
        strncpy(ifinfo_name, val, 63);  
        ifinfo_name[63] = '\0'; 
    }
    else if (!strncmp("precise", cmd, len))
    {
        char status[4];  // valid status is "on" or "off"
        strncpy(status, val, 3);
        status[3] = '\0';
        unsigned int len = strlen(status);
        for (unsigned int i = 0; i < len; i++)
            status[i] = tolower(status[i]);
        if (!strncmp("on", status, len))
        {
            dispatcher.SetPreciseTiming(true);
        }
        else if (!strncmp("off", status, len))
        {
            dispatcher.SetPreciseTiming(false);
        }
        else
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(precise) Error: invalid <status>\n");
            return false;
        }
    }
    else if (!strncmp("instance", cmd, len))
    {
        if (control_pipe.IsOpen())
            control_pipe.Close();
        if (control_pipe.Connect(val))
        {
            control_remote = true;
        }        
        else if (!control_pipe.Listen(val))
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(instance) error opening control pipe\n");
            return false; 
        }
    }
    else if (!strncmp("command", cmd, len))
    {
        if (!OpenCmdInput(val))
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(command) error opening command input file/device\n");
            return false;
        }
    }
#endif //OPNET
	
    else if (!strncmp("hostAddr", cmd, len))
    {
       char status[4];  // valid status is "on" or "off"
        strncpy(status, val, 3);
        status[3] = '\0';
        unsigned int len = strlen(status);
        for (unsigned int i = 0; i < len; i++)
            status[i] = tolower(status[i]);
        if (!strncmp("on", status, len))
        {
            // (TBD) control whither IPv4 or IPv6 ???
            ProtoAddress localAddress;
            if (localAddress.ResolveLocalAddress())
            {
                mgen.SetHostAddress(localAddress);   
            }
            else
            {
                DMSG(0, "OpnetMgenProcess::ProcessCommand(hostAddr) error getting local addr\n");
                FRET (false);
            }
        }
        else if (!strncmp("off", status, len))
        {
            mgen.ClearHostAddress();
        }
        else
        {
            DMSG(0, "OpnetMgenProcess::ProcessCommand(precise) Error: invalid <status>\n");
            FRET (false);
        }
    }
    else if (!strncmp("stop", cmd, len))
    {
       mgen.Stop();
    }
    else if (!strncmp("help", cmd, len))
    {
       fprintf(stderr, "mgen: version %s\n", MGEN_VERSION);
       //Usage();
       FRET (false);
    }
    FRET (true);
}  // end OpnetMgenProcess::OnCommand()



bool OpnetMgenProcess::OnStartup(int argc, const char*const* argv)
{ 
	FIN(OnStartup(argc,argv));
    // Seed the system rand() function
    struct timeval currentTime;
    ProtoSystemTime(currentTime);
    srand(currentTime.tv_usec);
	get_host_addr();
    
#ifdef HAVE_IPV6    
    if (ProtoSocket::HostIsIPv6Capable()) 
        mgen.SetDefaultSocketType(ProtoAddress::IPv6);
#endif // HAVE_IPV6
    
    mgen.SetPositionCallback(GetPosition, NULL);
	mgen_sink = new OpMgenSink(mgen,SINK);
	mgen_sink->Open();
	mgen_sink->SetSink(this);
	//sink_transport = MgenSinkTransport::Create(mgen);
    //sink_transport->SetSink(this);


#ifndef OPNET // JPH - no remote control for Opnet    
    if (!ProcessCommands(argc, argv))
    {
        fprintf(stderr, "mgen: error while processing startup commands\n");
        return false;
    }
    
    if (control_remote)
    {
        // We remoted commands, so we're finished ...
        return false;   
    }
#endif //OPNET

    fprintf(stderr, "mgen: version %s\n", MGEN_VERSION);
    
    if (convert)
    {
        fprintf(stderr, "mgen: beginning binary to text log conversion ...\n");
        MgenMsg theMsg;
        theMsg.ConvertBinaryLog(convert_path,mgen);
        fprintf(stderr, "mgen: conversion complete (exiting).\n");
    }
    else
    {
        if (mgen.DelayedStart())
        {
            char startTime[256];
            mgen.GetStartTime(startTime);
            fprintf(stderr, "mgen: delaying start until %s ...\n", startTime);
        }
        else
        {
            fprintf(stderr, "mgen: starting now ...\n");
        }
    }

#ifndef OPNET // JPH - not implemented for Opnet
    if ('\0' != ifinfo_name[0])
        GetInterfaceCounts(ifinfo_name, ifinfo_tx_count, ifinfo_rx_count);
    else
        ifinfo_tx_count = ifinfo_rx_count = 0; 
#endif  //OPNET
	
	/* Get command line */
	if (op_ima_obj_attr_exists(my_id,"Command")==OPC_TRUE)
		{
		char command[MAX_COMMAND];
		op_ima_obj_attr_get_str(my_id,"Command",MAX_COMMAND,command);
		if (command[0])
			{
			printf("%s: Command =  %s\n",node_name,command);
			mgen.ParseEvent(command,1);
			}
		}
	
	/* Get script file name */
	if (op_ima_obj_attr_exists(my_id,"Script")==OPC_TRUE)
		{
		char script[MAX_SCRIPT];
		op_ima_obj_attr_get_str(my_id,"Script",MAX_SCRIPT,script);
		if (script[0])
			{
			printf("%s: Script =  %s\n",node_name,script);
			mgen.ParseScript(script);
			}
		}
	
    FRET (mgen.Start());
}  // end MgenApp::OnStartup()

void OpnetMgenProcess::OnShutdown ()
	{
	mgen.Stop();
	}


bool OpnetMgenProcess::GetPosition (const void* agentPtr, GPSPosition& gpsPosition)
	{
	FIN (GetPosition(agentPtr,gpsPosition));
	op_ima_obj_attr_get (my_node_id, "x position", &gpsPosition.x);
	op_ima_obj_attr_get (my_node_id, "y position", &gpsPosition.y);
	op_ima_obj_attr_get (my_node_id, "altitude", &gpsPosition.z);
	gpsPosition.xyvalid = 1;
	gpsPosition.zvalid = 1;
	gpsPosition.tvalid = 0;
	gpsPosition.stale = 0;
	FRET (true);
	}


void OpnetMgenProcess::ReceivePacketMonitor(Ici* iciptr, Packet* pkptr)
	{
	FIN(ReceivePacketMonitor(iciptr,pkptr));
	/* Caclulate metrics to be updated.		*/
	double pk_size = (double) op_pk_total_size_get (pkptr);
	double ete_delay = op_sim_time () - op_pk_creation_time_get (pkptr);

	/* Update local statistics.				*/
	op_stat_write (bits_rcvd_stathandle, 		pk_size);
	op_stat_write (pkts_rcvd_stathandle, 		1.0);
	op_stat_write (ete_delay_stathandle, 		ete_delay);

	op_stat_write (bitssec_rcvd_stathandle, 	pk_size);
	op_stat_write (bitssec_rcvd_stathandle, 	0.0);
	op_stat_write (pktssec_rcvd_stathandle, 	1.0);
	op_stat_write (pktssec_rcvd_stathandle, 	0.0);

	/* Update global statistics.	*/
	op_stat_write (bits_rcvd_gstathandle, 		pk_size);
	op_stat_write (pkts_rcvd_gstathandle, 		1.0);
	op_stat_write (ete_delay_gstathandle, 		ete_delay);

	op_stat_write (bitssec_rcvd_gstathandle, 	pk_size);
	op_stat_write (bitssec_rcvd_gstathandle, 	0.0);
	op_stat_write (pktssec_rcvd_gstathandle, 	1.0);
	op_stat_write (pktssec_rcvd_gstathandle, 	0.0);

	char* buffer;
	MgenMsg msg;
	op_pk_fd_access_ptr(pkptr,0,(void**)&buffer);
	msg.Unpack(buffer,pk_size/8,false);

	unsigned int flowId = msg.GetFlowId();
	if (flowId<MAXSTATFLOWS)
		{
		op_stat_write (bitssec_rcvd_flow_stathandle[flowId], 	pk_size);
		op_stat_write (pktssec_rcvd_flow_stathandle[flowId], 	1.0);
		op_stat_write (ete_delay_flow_stathandle[flowId], 		ete_delay);
		tot_pkts_rcvd[flowId]++;
		pkts_rcvd[flowId]++;
		if (animFlag)
			{
			if (rx_did > -1)
				op_anim_igp_drawing_erase (olsr_vid, rx_did, OPC_ANIM_ERASE_MODE_XOR);
			if (op_ev_valid (rx_timeout_ev))
				op_ev_cancel (rx_timeout_ev);
			/* Draw flow id at receiving node's position. */
			char flowId_str[4];
			sprintf(flowId_str,"%d",flowId);
			rx_did = op_anim_igp_text_draw (olsr_vid, 
				OPC_ANIM_RETAIN | OPC_ANIM_PIXOP_XOR | OPC_ANIM_COLOR_ORANGE |
				OPC_ANIM_ALIGNH_RIGHT | OPC_ANIM_ALIGNV_CENTER,
				olsrNode[my_pnum]->vx, olsrNode[my_pnum]->vy+15, flowId_str);
			rx_timeout_ev = op_intrpt_schedule_self (op_sim_time () + TX_DISPLAY_TIME, RX_ANIM_TO);
			}
		}
	FOUT;
	}



void OpnetMgenProcess::TransmitPacketMonitor(Ici* ici, Packet* pkptr) 
	{
	FIN(TransmitPacketMonitor(ici,pkptr));
	/* Caclulate metrics to be updated.		*/
	double pk_size = (double) op_pk_total_size_get (pkptr);

	/* Update local statistics.				*/
	op_stat_write (bits_sent_stathandle, 		pk_size);
	op_stat_write (pkts_sent_stathandle, 		1.0);

	op_stat_write (bitssec_sent_stathandle, 	pk_size);
	op_stat_write (bitssec_sent_stathandle, 	0.0);
	op_stat_write (pktssec_sent_stathandle, 	1.0);
	op_stat_write (pktssec_sent_stathandle, 	0.0);

	/* Update global statistics.	*/
	op_stat_write (bits_sent_gstathandle, 		pk_size);
	op_stat_write (pkts_sent_gstathandle, 		1.0);

	op_stat_write (bitssec_sent_gstathandle, 	pk_size);
	op_stat_write (bitssec_sent_gstathandle, 	0.0);
	op_stat_write (pktssec_sent_gstathandle, 	1.0);
	op_stat_write (pktssec_sent_gstathandle, 	0.0);

	char* buffer;
	MgenMsg msg;
	op_pk_fd_access_ptr(pkptr,0,(void**)&buffer);
	msg.Unpack(buffer,pk_size/8,false);

	unsigned int flowId = msg.GetFlowId();
	if (flowId<MAXSTATFLOWS)
		{
		op_stat_write (bitssec_sent_flow_stathandle[flowId], 	pk_size);
		op_stat_write (pktssec_sent_flow_stathandle[flowId], 	1.0);
		tot_pkts_sent[flowId]++;
		pkts_sent[flowId]++;
		if (animFlag)
			{
			if (tx_did > -1)
				op_anim_igp_drawing_erase (olsr_vid, tx_did, OPC_ANIM_ERASE_MODE_XOR);
			if (op_ev_valid (tx_timeout_ev))
				op_ev_cancel (tx_timeout_ev);
			/* Draw flow id at transmitting node's position. */
			char flowId_str[4];
			sprintf(flowId_str,"%d",flowId);
			tx_did = op_anim_igp_text_draw (olsr_vid, 
				OPC_ANIM_RETAIN | OPC_ANIM_PIXOP_XOR | OPC_ANIM_COLOR_BLUE |
				OPC_ANIM_ALIGNH_LEFT | OPC_ANIM_ALIGNV_CENTER,
				olsrNode[my_pnum]->vx, olsrNode[my_pnum]->vy+15, flowId_str);
			tx_timeout_ev = op_intrpt_schedule_self (op_sim_time () + TX_DISPLAY_TIME, TX_ANIM_TO);
			}
		}
	FOUT;
	}


MgenSinkTransport* MgenSinkTransport::Create(Mgen& theMgen)
{
    return static_cast<MgenSinkTransport*>(new OpMgenSink(theMgen,SINK));
}




OpMgenSink::OpMgenSink(Mgen& theMgen,Protocol theProtocol)
  : MgenSinkTransport(theMgen,theProtocol)
{
}

OpMgenSink::~OpMgenSink()
{
}

bool OpMgenSink::SendMessage(MgenMsg& theMsg,
                             const ProtoAddress& dst_addr,
                             char* txBuffer)
	{
	FIN(SendMessage(theMsg,dst_addr,txBuffer));
	UINT32 txChecksum = 0;
	unsigned int len = 0;
	theMsg.SetFlag(MgenMsg::LAST_BUFFER);

	len = theMsg.Pack(txBuffer,theMsg.GetMsgLen(),mgen.GetChecksumEnable(),txChecksum);

	if (len == 0)
		FRET (false); // no room

	if (mgen.GetChecksumEnable() && theMsg.FlagIsSet(MgenMsg::CHECKSUM))
		theMsg.WriteChecksum(txChecksum,(unsigned char*)txBuffer,(UINT32)len);

	//if (!msg_sink->HandleMessage(txBuffer,theMsg.GetMsgLen(),theMsg.GetSrcAddr()))
		//return false;

	//MgenTransport::LogEvent(SEND_EVENT,&theMsg,theMsg.GetTxTime(),txBuffer);

    Packet* pkt = op_pk_create(len*8);
    char* payload = (char*) op_prg_mem_copy_create((void*)txBuffer, len);
    op_pk_fd_set(pkt, 0, OPC_FIELD_TYPE_STRUCT, payload, 0,
                 op_prg_mem_copy_create, op_prg_mem_free, len);
	//op_ici_attr_set(norm_ici, "local_port", proto_socket->GetPort());
    op_ici_attr_set(sink_ici, "rem_port", dst_addr.GetPort());
    op_ici_attr_set(sink_ici, "rem_addr", dst_addr.SimGetAddress());
    op_ici_install(sink_ici);
    
    mgen_proc.TransmitPacketMonitor(sink_ici, pkt);

    op_pk_send_forced(pkt, 2);
    FRET (true);
	
	}
           
bool OpMgenSink::Open()
	{
	FIN(Open());
	sink_ici = op_ici_create("sink_command");
	FRET (sink_ici != OPC_NIL);
	}

void OpMgenSink::Close()
	{
	FIN (Close());
	//op_ici_destroy(sink_ici);
	FOUT;
	}


// ProtoMessageSink override

OpnetProtoMessageSink::OpnetProtoMessageSink(Ici* iciptr, int ostrm, int port)
{
	app_ici_ptr = iciptr;
	outstrm_to_mgen = ostrm;
	loc_port = port;
}


OpnetProtoMessageSink::OpnetProtoMessageSink()
{
}

OpnetProtoMessageSink::~OpnetProtoMessageSink()
{
}


bool OpnetProtoMessageSink::HandleMessage(const char*   buffer, 
                           unsigned int      buflen, 
                           const ProtoAddress& srcAddr)
	{
	FIN (HandleMessage(buffer,buflen,srcAddr));
	Packet* pkt = op_pk_create(buflen*8);
    char* payload = (char*) op_prg_mem_copy_create((void*)buffer, buflen);
    op_pk_fd_set(pkt, 0, OPC_FIELD_TYPE_STRUCT, payload, 0,
                 op_prg_mem_copy_create, op_prg_mem_free, buflen);
	
	
	op_ici_attr_set(app_ici_ptr, "local_port", loc_port);
    op_ici_attr_set(app_ici_ptr, "rem_port", srcAddr.GetPort());
    op_ici_attr_set(app_ici_ptr, "rem_addr", srcAddr.SimGetAddress());
    op_ici_attr_set(app_ici_ptr, "src_addr", srcAddr.SimGetAddress());
    op_ici_install(app_ici_ptr);
  	
    op_pk_send_forced(pkt, outstrm_to_mgen);
	FRET(true);
	}

/* JPH 11/14/06 - packet animation */

void rx_erase()
	{
	FIN(rx_erase());
	op_anim_igp_drawing_erase (olsr_vid, rx_did, OPC_ANIM_ERASE_MODE_XOR);
	rx_did = -1;
	FOUT;
	}

void tx_erase()
	{
	FIN(tx_erase());
	op_anim_igp_drawing_erase (olsr_vid, tx_did, OPC_ANIM_ERASE_MODE_XOR);
	tx_did = -1;
	FOUT;
	}

/* JPH end */

/* End of Function Block */

/* Undefine optional tracing in FIN/FOUT/FRET */
/* The FSM has its own tracing code and the other */
/* functions should not have any tracing.		  */
#undef FIN_TRACING
#define FIN_TRACING

#undef FOUTRET_TRACING
#define FOUTRET_TRACING

/* Undefine shortcuts to state variables because the */
/* following functions are part of the state class */
#undef mgen_proc
#undef mgen_sink
#undef my_id
#undef my_node_id
#undef my_pro_id
#undef my_udp_id
#undef my_tcp_id
#undef my_ip_addr
#undef my_ip_mask
#undef own_prohandle
#undef own_process_record_handle
#undef pid_string
#undef node_name
#undef timer
#undef udpnotifier
#undef tcpnotifier
#undef host_ipv4_addr
#undef bits_rcvd_stathandle
#undef bitssec_rcvd_flow_stathandle
#undef bitssec_sent_flow_stathandle
#undef bitssec_rcvd_stathandle
#undef pkts_rcvd_stathandle
#undef pktssec_rcvd_flow_stathandle
#undef pktssec_sent_flow_stathandle
#undef pktssec_rcvd_stathandle
#undef bits_sent_stathandle
#undef bitssec_sent_stathandle
#undef pkts_sent_stathandle
#undef pktssec_sent_stathandle
#undef ete_delay_stathandle
#undef ete_delay_flow_stathandle
#undef bits_rcvd_gstathandle
#undef bitssec_rcvd_gstathandle
#undef pkts_rcvd_gstathandle
#undef pktssec_rcvd_gstathandle
#undef bits_sent_gstathandle
#undef bitssec_sent_gstathandle
#undef pkts_sent_gstathandle
#undef pktssec_sent_gstathandle
#undef ete_delay_gstathandle
#undef udp_outstream_index
#undef tcp_outstream_index
#undef norm_outstream_index
#undef local_port
#undef script_fp
#undef sink_ici
#undef pkts_sent
#undef pkts_rcvd
#undef tx_did
#undef rx_did
#undef my_pnum
#undef tx_timeout_ev
#undef rx_timeout_ev

/* Access from C kernel using C linkage */
extern "C"
{
	VosT_Obtype _op_mgen_protolib_init (int * init_block_ptr);
	VosT_Address _op_mgen_protolib_alloc (VosT_Obtype, int);
	void mgen_protolib (OP_SIM_CONTEXT_ARG_OPT)
		{
		((mgen_protolib_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr))->mgen_protolib (OP_SIM_CONTEXT_PTR_OPT);
		}

	void _op_mgen_protolib_svar (void *, const char *, void **);

	void _op_mgen_protolib_diag (OP_SIM_CONTEXT_ARG_OPT)
		{
		((mgen_protolib_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr))->_op_mgen_protolib_diag (OP_SIM_CONTEXT_PTR_OPT);
		}

	void _op_mgen_protolib_terminate (OP_SIM_CONTEXT_ARG_OPT)
		{
		/* The destructor is the Termination Block */
		delete (mgen_protolib_state *)(OP_SIM_CONTEXT_PTR->_op_mod_state_ptr);
		}


	VosT_Obtype Vos_Define_Object_Prstate (const char * _op_name, size_t _op_size);
	VosT_Address Vos_Alloc_Object (VosT_Obtype _op_ob_hndl);
	VosT_Fun_Status Vos_Poolmem_Dealloc (VosT_Address _op_ob_ptr);
} /* end of 'extern "C"' */




/* Process model interrupt handling procedure */


void
mgen_protolib_state::mgen_protolib (OP_SIM_CONTEXT_ARG_OPT)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	FIN_MT (mgen_protolib_state::mgen_protolib ());
	try
		{
		/* Temporary Variables */
		
		/* End of Temporary Variables */


		FSM_ENTER ("mgen_protolib")

		FSM_BLOCK_SWITCH
			{
			/*---------------------------------------------------------*/
			/** state (idle) enter executives **/
			FSM_STATE_ENTER_UNFORCED (0, "idle", state0_enter_exec, "mgen_protolib [idle enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (1,"mgen_protolib")


			/** state (idle) exit executives **/
			FSM_STATE_EXIT_UNFORCED (0, "idle", "mgen_protolib [idle exit execs]")


			/** state (idle) transition processing **/
			FSM_PROFILE_SECTION_IN ("mgen_protolib [idle trans conditions]", state0_trans_conds)
			FSM_INIT_COND (END_SIM)
			FSM_TEST_COND (MSG_ARRIVAL)
			FSM_TEST_COND (TIMEOUT_EVENT)
			FSM_TEST_COND (TCP_IND)
			FSM_TEST_COND (RX_TO)
			FSM_TEST_COND (TX_TO)
			FSM_DFLT_COND
			FSM_TEST_LOGIC ("idle")
			FSM_PROFILE_SECTION_OUT (state0_trans_conds)

			FSM_TRANSIT_SWITCH
				{
				FSM_CASE_TRANSIT (0, 0, state0_enter_exec, stop();, "END_SIM", "stop()", "idle", "idle", "mgen_protolib [idle -> idle : END_SIM / stop()]")
				FSM_CASE_TRANSIT (1, 3, state3_enter_exec, ;, "MSG_ARRIVAL", "", "idle", "proc_msg", "mgen_protolib [idle -> proc_msg : MSG_ARRIVAL / ]")
				FSM_CASE_TRANSIT (2, 4, state4_enter_exec, ;, "TIMEOUT_EVENT", "", "idle", "itimer", "mgen_protolib [idle -> itimer : TIMEOUT_EVENT / ]")
				FSM_CASE_TRANSIT (3, 5, state5_enter_exec, ;, "TCP_IND", "", "idle", "tcp_ind", "mgen_protolib [idle -> tcp_ind : TCP_IND / ]")
				FSM_CASE_TRANSIT (4, 0, state0_enter_exec, rx_erase();, "RX_TO", "rx_erase()", "idle", "idle", "mgen_protolib [idle -> idle : RX_TO / rx_erase()]")
				FSM_CASE_TRANSIT (5, 0, state0_enter_exec, tx_erase();, "TX_TO", "tx_erase()", "idle", "idle", "mgen_protolib [idle -> idle : TX_TO / tx_erase()]")
				FSM_CASE_TRANSIT (6, 0, state0_enter_exec, ;, "default", "", "idle", "idle", "mgen_protolib [idle -> idle : default / ]")
				}
				/*---------------------------------------------------------*/



			/** state (init) enter executives **/
			FSM_STATE_ENTER_UNFORCED_NOLABEL (1, "init", "mgen_protolib [init enter execs]")
				FSM_PROFILE_SECTION_IN ("mgen_protolib [init enter execs]", state1_enter_exec)
				{
				
				/* Obtain the object ID of the surrounding mgen processor. 	*/
				my_id = op_id_self ();
				
				/* Also obtain the object ID of the surrounding node.		*/
				my_node_id = op_topo_parent (my_id);
				
				/* Obtain the prohandle for this process.					*/
				own_prohandle = op_pro_self ();
				
				/**	Register the process in the model-wide registry.				**/
				own_process_record_handle = (OmsT_Pr_Handle) oms_pr_process_register 
					(my_node_id, my_id, own_prohandle, "MGEN");
				
				/*	Register the protocol attribute in the registry. No other	*/
				/*	process should use the string "mgen" as the value for its	*/
				/*	"protocol" attribute!										*/
				oms_pr_attr_set (own_process_record_handle, 
					"protocol", 	OMSC_PR_STRING, 	"mgen",
					OPC_NIL);
				
				/*	Initialize the state variable used to keep track of the	*/
				/*	MGEN module object ID and to generate trace/debugging 	*/
				/*	string information. Obtain process ID of this process. 	*/
				my_pro_id = op_pro_id (op_pro_self ());
				
				/* 	Set the process ID string, to be later used for trace	*/
				/*	and debugging information.								*/
				sprintf (pid_string, "MGEN PID (%d)", my_pro_id);
				
				/* Get the name of the surrounding node object */
				op_ima_obj_attr_get (my_node_id, "name", &node_name);
				
				/* 	Schedule a self interrupt to allow  additional initialization. */
				op_intrpt_schedule_self (op_sim_time (), 0);
				
				
				/* JPH - packet animation */
				if (!animInit)
					{
					animInit = OPC_TRUE;  // Animation initialization that is only done once.
					for (int i=0; i<MAX_MGEN_PORTS; i++)
						mgen_tcp_port[i] = mgen_udp_port[i] = -1;
					}
				}
				FSM_PROFILE_SECTION_OUT (state1_enter_exec)

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (3,"mgen_protolib")


			/** state (init) exit executives **/
			FSM_STATE_EXIT_UNFORCED (1, "init", "mgen_protolib [init exit execs]")
				FSM_PROFILE_SECTION_IN ("mgen_protolib [init exit execs]", state1_exit_exec)
				{
				mgen_init();
				op_intrpt_schedule_self (0.0,0);
				}
				FSM_PROFILE_SECTION_OUT (state1_exit_exec)


			/** state (init) transition processing **/
			FSM_TRANSIT_FORCE (2, state2_enter_exec, ;, "default", "", "init", "init2", "mgen_protolib [init -> init2 : default / ]")
				/*---------------------------------------------------------*/



			/** state (init2) enter executives **/
			FSM_STATE_ENTER_UNFORCED (2, "init2", state2_enter_exec, "mgen_protolib [init2 enter execs]")

			/** blocking after enter executives of unforced state. **/
			FSM_EXIT (5,"mgen_protolib")


			/** state (init2) exit executives **/
			FSM_STATE_EXIT_UNFORCED (2, "init2", "mgen_protolib [init2 exit execs]")
				FSM_PROFILE_SECTION_IN ("mgen_protolib [init2 exit execs]", state2_exit_exec)
				{
				mgen_proc.OnStartup(0,NULL);
				if (animFlag)
					{
					int *pnum_ptr;
					pnum_ptr = (int*)op_ima_obj_state_get (my_node_id);
					my_pnum = *pnum_ptr;
					}
				}
				FSM_PROFILE_SECTION_OUT (state2_exit_exec)


			/** state (init2) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "init2", "idle", "mgen_protolib [init2 -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (proc_msg) enter executives **/
			FSM_STATE_ENTER_FORCED (3, "proc_msg", state3_enter_exec, "mgen_protolib [proc_msg enter execs]")
				FSM_PROFILE_SECTION_IN ("mgen_protolib [proc_msg enter execs]", state3_enter_exec)
				{
				mgen_proc.OnReceive(op_intrpt_strm(),node_name);
				}
				FSM_PROFILE_SECTION_OUT (state3_enter_exec)

			/** state (proc_msg) exit executives **/
			FSM_STATE_EXIT_FORCED (3, "proc_msg", "mgen_protolib [proc_msg exit execs]")


			/** state (proc_msg) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "proc_msg", "idle", "mgen_protolib [proc_msg -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (itimer) enter executives **/
			FSM_STATE_ENTER_FORCED (4, "itimer", state4_enter_exec, "mgen_protolib [itimer enter execs]")
				FSM_PROFILE_SECTION_IN ("mgen_protolib [itimer enter execs]", state4_enter_exec)
				{
				mgen_proc.OnSystemTimeout();
				}
				FSM_PROFILE_SECTION_OUT (state4_enter_exec)

			/** state (itimer) exit executives **/
			FSM_STATE_EXIT_FORCED (4, "itimer", "mgen_protolib [itimer exit execs]")


			/** state (itimer) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "itimer", "idle", "mgen_protolib [itimer -> idle : default / ]")
				/*---------------------------------------------------------*/



			/** state (tcp_ind) enter executives **/
			FSM_STATE_ENTER_FORCED (5, "tcp_ind", state5_enter_exec, "mgen_protolib [tcp_ind enter execs]")
				FSM_PROFILE_SECTION_IN ("mgen_protolib [tcp_ind enter execs]", state5_enter_exec)
				{
				Ici* iciPtr = op_intrpt_ici();
				char iciformat[32];
				int connId;
				int status;
				int urgent;
				int pcnt;
				
				op_ici_format(iciPtr,iciformat);
				if (!strcmp("tcp_open_ind",iciformat))
					mgen_proc.OnAccept(node_name);
				else if (!strcmp("tcp_status_ind",iciformat))
				{
					op_ici_attr_get (iciPtr, "conn_id", &connId);
					op_ici_attr_get (iciPtr, "status", &status);
					op_ici_attr_get (iciPtr, "urgent", &urgent);
					op_ici_attr_get (iciPtr, "packet count", &pcnt);
					printf("node: %s  time: %f   %s  connId: %d  status: %d  urgent: %d  packet count: %d\n",
					node_name,op_sim_time(),iciformat,connId,status,urgent,pcnt);
				}
				}
				FSM_PROFILE_SECTION_OUT (state5_enter_exec)

			/** state (tcp_ind) exit executives **/
			FSM_STATE_EXIT_FORCED (5, "tcp_ind", "mgen_protolib [tcp_ind exit execs]")


			/** state (tcp_ind) transition processing **/
			FSM_TRANSIT_FORCE (0, state0_enter_exec, ;, "default", "", "tcp_ind", "idle", "mgen_protolib [tcp_ind -> idle : default / ]")
				/*---------------------------------------------------------*/



			}


		FSM_EXIT (1,"mgen_protolib")
		}
	catch (...)
		{
		Vos_Error_Print (VOSC_ERROR_ABORT,
			(const char *)VOSC_NIL,
			"Unhandled C++ exception in process model (mgen_protolib)",
			(const char *)VOSC_NIL, (const char *)VOSC_NIL);
		}
	}




void
mgen_protolib_state::_op_mgen_protolib_diag (OP_SIM_CONTEXT_ARG_OPT)
	{
	/* No Diagnostic Block */
	}

void
mgen_protolib_state::operator delete (void* ptr)
	{
	FIN (mgen_protolib_state::operator delete (ptr));
	Vos_Poolmem_Dealloc (ptr);
	FOUT
	}

mgen_protolib_state::~mgen_protolib_state (void)
	{

	FIN (mgen_protolib_state::~mgen_protolib_state ())


	/* No Termination Block */


	FOUT
	}


#undef FIN_PREAMBLE_DEC
#undef FIN_PREAMBLE_CODE

#define FIN_PREAMBLE_DEC
#define FIN_PREAMBLE_CODE

void *
mgen_protolib_state::operator new (size_t)
#if defined (VOSD_NEW_BAD_ALLOC)
		throw (VOSD_BAD_ALLOC)
#endif
	{
	void * new_ptr;

	FIN_MT (mgen_protolib_state::operator new ());

	new_ptr = Vos_Alloc_Object (mgen_protolib_state::obtype);
#if defined (VOSD_NEW_BAD_ALLOC)
	if (new_ptr == VOSC_NIL) throw VOSD_BAD_ALLOC();
#endif
	FRET (new_ptr)
	}

/* State constructor initializes FSM handling */
/* by setting the initial state to the first */
/* block of code to enter. */

mgen_protolib_state::mgen_protolib_state (void) :
		_op_current_block (2)
	{
#if defined (OPD_ALLOW_ODB)
		_op_current_state = "mgen_protolib [init enter execs]";
#endif
	}

VosT_Obtype
_op_mgen_protolib_init (int * init_block_ptr)
	{
	FIN_MT (_op_mgen_protolib_init (init_block_ptr))

	mgen_protolib_state::obtype = Vos_Define_Object_Prstate ("proc state vars (mgen_protolib)",
		sizeof (mgen_protolib_state));
	*init_block_ptr = 2;

	FRET (mgen_protolib_state::obtype)
	}

VosT_Address
_op_mgen_protolib_alloc (VosT_Obtype, int)
	{
#if !defined (VOSD_NO_FIN)
	int _op_block_origin = 0;
#endif
	mgen_protolib_state * ptr;
	FIN_MT (_op_mgen_protolib_alloc ())

	/* New instance will have FSM handling initialized */
#if defined (VOSD_NEW_BAD_ALLOC)
	try {
		ptr = new mgen_protolib_state;
	} catch (const VOSD_BAD_ALLOC &) {
		ptr = VOSC_NIL;
	}
#else
	ptr = new mgen_protolib_state;
#endif
	FRET ((VosT_Address)ptr)
	}



void
_op_mgen_protolib_svar (void * gen_ptr, const char * var_name, void ** var_p_ptr)
	{
	mgen_protolib_state		*prs_ptr;

	FIN_MT (_op_mgen_protolib_svar (gen_ptr, var_name, var_p_ptr))

	if (var_name == OPC_NIL)
		{
		*var_p_ptr = (void *)OPC_NIL;
		FOUT
		}
	prs_ptr = (mgen_protolib_state *)gen_ptr;

	if (strcmp ("mgen_proc" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->mgen_proc);
		FOUT
		}
	if (strcmp ("mgen_sink" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->mgen_sink);
		FOUT
		}
	if (strcmp ("my_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_id);
		FOUT
		}
	if (strcmp ("my_node_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_node_id);
		FOUT
		}
	if (strcmp ("my_pro_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_pro_id);
		FOUT
		}
	if (strcmp ("my_udp_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_udp_id);
		FOUT
		}
	if (strcmp ("my_tcp_id" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_tcp_id);
		FOUT
		}
	if (strcmp ("my_ip_addr" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_ip_addr);
		FOUT
		}
	if (strcmp ("my_ip_mask" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_ip_mask);
		FOUT
		}
	if (strcmp ("own_prohandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->own_prohandle);
		FOUT
		}
	if (strcmp ("own_process_record_handle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->own_process_record_handle);
		FOUT
		}
	if (strcmp ("pid_string" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->pid_string);
		FOUT
		}
	if (strcmp ("node_name" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->node_name);
		FOUT
		}
	if (strcmp ("timer" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->timer);
		FOUT
		}
	if (strcmp ("udpnotifier" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->udpnotifier);
		FOUT
		}
	if (strcmp ("tcpnotifier" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->tcpnotifier);
		FOUT
		}
	if (strcmp ("host_ipv4_addr" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->host_ipv4_addr);
		FOUT
		}
	if (strcmp ("bits_rcvd_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->bits_rcvd_stathandle);
		FOUT
		}
	if (strcmp ("bitssec_rcvd_flow_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->bitssec_rcvd_flow_stathandle);
		FOUT
		}
	if (strcmp ("bitssec_sent_flow_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->bitssec_sent_flow_stathandle);
		FOUT
		}
	if (strcmp ("bitssec_rcvd_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->bitssec_rcvd_stathandle);
		FOUT
		}
	if (strcmp ("pkts_rcvd_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pkts_rcvd_stathandle);
		FOUT
		}
	if (strcmp ("pktssec_rcvd_flow_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->pktssec_rcvd_flow_stathandle);
		FOUT
		}
	if (strcmp ("pktssec_sent_flow_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->pktssec_sent_flow_stathandle);
		FOUT
		}
	if (strcmp ("pktssec_rcvd_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pktssec_rcvd_stathandle);
		FOUT
		}
	if (strcmp ("bits_sent_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->bits_sent_stathandle);
		FOUT
		}
	if (strcmp ("bitssec_sent_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->bitssec_sent_stathandle);
		FOUT
		}
	if (strcmp ("pkts_sent_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pkts_sent_stathandle);
		FOUT
		}
	if (strcmp ("pktssec_sent_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pktssec_sent_stathandle);
		FOUT
		}
	if (strcmp ("ete_delay_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->ete_delay_stathandle);
		FOUT
		}
	if (strcmp ("ete_delay_flow_stathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->ete_delay_flow_stathandle);
		FOUT
		}
	if (strcmp ("bits_rcvd_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->bits_rcvd_gstathandle);
		FOUT
		}
	if (strcmp ("bitssec_rcvd_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->bitssec_rcvd_gstathandle);
		FOUT
		}
	if (strcmp ("pkts_rcvd_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pkts_rcvd_gstathandle);
		FOUT
		}
	if (strcmp ("pktssec_rcvd_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pktssec_rcvd_gstathandle);
		FOUT
		}
	if (strcmp ("bits_sent_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->bits_sent_gstathandle);
		FOUT
		}
	if (strcmp ("bitssec_sent_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->bitssec_sent_gstathandle);
		FOUT
		}
	if (strcmp ("pkts_sent_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pkts_sent_gstathandle);
		FOUT
		}
	if (strcmp ("pktssec_sent_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->pktssec_sent_gstathandle);
		FOUT
		}
	if (strcmp ("ete_delay_gstathandle" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->ete_delay_gstathandle);
		FOUT
		}
	if (strcmp ("udp_outstream_index" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->udp_outstream_index);
		FOUT
		}
	if (strcmp ("tcp_outstream_index" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->tcp_outstream_index);
		FOUT
		}
	if (strcmp ("norm_outstream_index" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->norm_outstream_index);
		FOUT
		}
	if (strcmp ("local_port" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->local_port);
		FOUT
		}
	if (strcmp ("script_fp" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->script_fp);
		FOUT
		}
	if (strcmp ("sink_ici" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->sink_ici);
		FOUT
		}
	if (strcmp ("pkts_sent" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->pkts_sent);
		FOUT
		}
	if (strcmp ("pkts_rcvd" , var_name) == 0)
		{
		*var_p_ptr = (void *) (prs_ptr->pkts_rcvd);
		FOUT
		}
	if (strcmp ("tx_did" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->tx_did);
		FOUT
		}
	if (strcmp ("rx_did" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->rx_did);
		FOUT
		}
	if (strcmp ("my_pnum" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->my_pnum);
		FOUT
		}
	if (strcmp ("tx_timeout_ev" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->tx_timeout_ev);
		FOUT
		}
	if (strcmp ("rx_timeout_ev" , var_name) == 0)
		{
		*var_p_ptr = (void *) (&prs_ptr->rx_timeout_ev);
		FOUT
		}
	*var_p_ptr = (void *)OPC_NIL;

	FOUT
	}

