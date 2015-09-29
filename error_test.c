#include <string.h>
#include "spin1_api.h"
#include "jtag.h"

#define NUMCORES 16
#define READREPS 10
#define SDRAM_BUFFER 1750000 // 7000000 32-bit words = 28 MBytes per core = 112 MBytes (4 cores)
#define SPIN3
#define BOARD_STEP_IP 16

// default values for 1 Spin5 board
#ifdef SPIN3
  #define XCHIPS_BOARD 2
  #define YCHIPS_BOARD 2

  #define XCHIPS 2
  #define YCHIPS 2
#endif


void allocate_memory(void);
uint *mem_alloc(uint buffer_size);
void ijtag_init(void);
void gen_random_data(void);
void read_sdram(void);

void app_done(void);
void sdp_init(void);
uint spin1_get_chip_board_id(void);
uint spin1_get_eth_board_id(void);
uchar *spin1_get_ipaddr(void);

// Global variables
uint coreID;
uint chipID, chipBoardID, ethBoardID, boardNum;
uchar boardIP[4];
uint chipIDx, chipIDy, chipNum, chipBoardIDx, chipBoardIDy, chipBoardNum;
uint *sdram_data;
sdp_msg_t my_msg;

int c_main(void)
{
  // Reset JTAG controller if leadAp
  if (leadAp)
    ijtag_init();

  // Get core and chip IDs
  coreID      = spin1_get_core_id();
  chipID      = spin1_get_chip_id();
  chipBoardID = spin1_get_chip_board_id();
  ethBoardID  = spin1_get_eth_board_id();
  //boardIP     = spin1_get_ipaddr();
  
  boardIP[0]  = (uint)sv->ip_addr[0];
  boardIP[1]  = (uint)sv->ip_addr[1];
  boardIP[2]  = (uint)sv->ip_addr[2];
  boardIP[3]  = (uint)sv->ip_addr[3];
  boardNum    = (boardIP[3]-1)/BOARD_STEP_IP;

  // get this chip's coordinates for core map
  chipIDx = chipID>>8;
  chipIDy = chipID&255;
  chipNum = (chipIDy * YCHIPS) + chipIDx;

  chipBoardIDx = chipBoardID>>8;
  chipBoardIDy = chipBoardID&255;
  chipBoardNum = (chipBoardIDy * YCHIPS_BOARD) + chipBoardIDx;

  // initialise SDP message buffer
  sdp_init();

  // Allocate SDRAM memory for the original, encoded and decoded arrays
  allocate_memory();

  // Generate random data
  gen_random_data();

  // Read data
  for(uint i=0; i<READREPS; i++)
  	read_sdram();
  //spin1_exit(0);

  // Go
  //spin1_start(SYNC_WAIT);

  // report results
  //app_done();

  return 0;

}

// Reset JTAG controller
void ijtag_init(void)
{
   // reset the jtag signals
   sc[GPIO_CLR] = JTAG_TDI + JTAG_TCK + JTAG_TMS + JTAG_NTRST;

   // select internal jtag signals
   sc[SC_MISC_CTRL] |= JTAG_INT;
}

// Initialise SDP
void sdp_init(void)
{
  my_msg.tag       = 1;             // IPTag 1
  my_msg.dest_port = PORT_ETH;      // Ethernet
  my_msg.dest_addr = sv->eth_addr;  // Eth connected chip on this board

  my_msg.flags     = 0x07;          // Flags = 7
  my_msg.srce_port = spin1_get_core_id ();  // Source port
  my_msg.srce_addr = spin1_get_chip_id ();  // Source addr
}

// Function that reports the chip address relative to the bottom.
uint spin1_get_chip_board_id(void)
{
  return (uint)sv->board_addr;
}

// Function that reports the address of the nearest ethernet chip
uint spin1_get_eth_board_id(void)
{
  return (uint)sv->eth_addr;
}

// Function that reports the IP address of the current board.
uchar *spin1_get_ipaddr(void)
{
  return sv->ip_addr;
}

// Allocate the SDRAM memory for the transmit as well as the receive chips
void allocate_memory(void)
{
  // Allocate memory
  if (coreID>=1 && coreID<=NUMCORES)
    sdram_data = mem_alloc(SDRAM_BUFFER);
}

// Allocate and initialise memory
uint *mem_alloc(uint buffer_size)
{
  uint *data;

  if (!(data = (uint *)sark_xalloc (sv->sdram_heap, buffer_size*sizeof(uint), 0, ALLOC_LOCK)))
  {
    io_printf(IO_BUF, "Unable to allocate memory!\n");
    rt_error(RTE_ABORT);
  } 

  return data;
}


// Generate the random data array for the transmit chips
void gen_random_data(void)
{
  if (coreID>=1 && coreID<=NUMCORES)
  {
    //Seed random number generator
    //sark_srand(chipID + coreID);
    sark_srand(35);

    //Initialize buffer
    for(uint i=0; i<SDRAM_BUFFER; i++)
    {
      sdram_data[i] = sark_rand();

      if (i%10000==0)
      	io_printf(IO_BUF, "Progress: %d\n", i);
    }
  }
}

void read_sdram(void)
{
	if (coreID>=1 &7 coreID<=NUMCORES)
	{

	}
}

void config_DMA(void)
{

offset 0x0180

0xFB808B20
0x7DC04590
0xBEE022C8
0x5F701164
0x2FB808B2
0x97DC0459
0xB06E890C
0x58374486
0xAC1BA243
0xAD8D5A01
0xAD462620
0x56A31310
0x2B518988
0x95A8C4C4
0xCAD46262
0x656A3131
0x493593B8
0x249AC9DC
0x924D64EE
0xC926B277
0x9F13D21B
0xB409622D
0x21843A36
0x90C21D1B
0x33E185AD
0x627049F6
0x313824FB
0xE31C995D
0x8A0EC78E
0xC50763C7
0x19033AC3
0xF7011641

}

// Function that reports the total simulation time.
void app_done(void)
{
  // report simulation time
  if(coreID>=1 && coreID<=NUMCORES)
    io_printf(IO_BUF, "Sim lasted %d ticks.\n\n", spin1_get_simulation_time());
}

