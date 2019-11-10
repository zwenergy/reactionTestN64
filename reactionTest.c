#include <libdragon.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ROWSPACE 15
#define COLSPACE 40
#define COORDXINIT 10
#define COORDYINIT 10
#define SELECTSPACE 8
#define MINWAITMS 1000
#define MAXWAITMS 5000

//#define DEBUG

#define TICKTOMS_DOUBLE( tck ) ( ( (double) tck * ( 1000.0 / 46875.0 ) ) / 1000.0 )
#define TICKTOMS( tck ) ( TIMER_MICROS_LL( tck ) / 1000 )


enum ePoll { ZeroMS = 0, FiveMS, TenMS, TwentyMS, ThirtyMS };
enum eRep { rep1 = 0, rep5, rep10, rep20, rep30 };
enum eChoice { poll, rep };

uint32_t blackCol;
uint32_t whiteCol;
uint32_t selCol;

uint32_t butPressShow = 0;
uint32_t butAPrev = 0;
uint32_t butSucc = 0;

// Arrays for storing the latencies.
unsigned long long latTicks[ 30 ];

// The following function checks if A was pressed if A was supposed to
// be pressed.
void checkButton( int ovfl ) {
  // Avoid warning.
  (void) ovfl;
  // Check if we registered a button down press.
  controller_scan();
  struct controller_data cont = get_keys_pressed();
  
  // Do nothing if nothing was shown yet.
  if ( !butPressShow ) {
    butSucc = 0;
    butAPrev = cont.c[0].A;
    return;
  }
  

  
  if ( cont.c[0].A && !butAPrev ) {
    // Success.
    butSucc = 1;
  }
  
  // Store the old value.
  butAPrev = cont.c[0].A;
  
  return;
}

void mainMenu() {
  enum eChoice curCat = poll;
  enum eRep curRep = rep1;
  enum ePoll curPoll = ZeroMS;
    
  // We store the option coordinates.
  unsigned int coordPollY;
  unsigned int coordPollX[ 5 ];
  unsigned int coordRepY;
  unsigned int coordRepX[ 5 ];
  
  // Hold prev. buttons.
  struct controller_data prevCont;
  controller_scan();
  prevCont = get_keys_pressed();
  
  while( 1 ) {
    int curX = COORDXINIT;
    int curY = COORDYINIT;
    // Get a render lock.
    display_context_t disp = 0;
    while( !( disp = display_lock() ) ) {};
    
    // Set colors.
    graphics_set_color( whiteCol, blackCol );
    
    // Print the title.
    graphics_fill_screen( disp, 0 );
    graphics_draw_text( disp, curX, curY, "================================" );
    curY += ROWSPACE;
    graphics_draw_text( disp, curX, curY, "    reactionTest by zwenergy    " );
    curY += ROWSPACE;
    graphics_draw_text( disp, curX, curY, "================================" );
    
    curY += 2*ROWSPACE;
    graphics_draw_text( disp, curX, curY, "Polling time:" );
    curY += ROWSPACE;
    graphics_draw_text( disp, curX, curY, "0ms" );
    coordPollY = curY;
    coordPollX[ 0 ] = curX - SELECTSPACE;
    
    curX += COLSPACE;
    graphics_draw_text( disp, curX, curY, "5ms" );
    coordPollX[ 1 ] = curX - SELECTSPACE;
    
    curX += COLSPACE;
    graphics_draw_text( disp, curX, curY, "10ms" );
    coordPollX[ 2 ] = curX - SELECTSPACE;
    
    curX += COLSPACE;
    graphics_draw_text( disp, curX, curY, "20ms" );
    coordPollX[ 3 ] = curX - SELECTSPACE;
    
    curX += COLSPACE;
    graphics_draw_text( disp, curX, curY, "30ms" );
    coordPollX[ 4 ] = curX - SELECTSPACE;
    
    curX = COORDXINIT;
    curY += 2*ROWSPACE;
    
    graphics_draw_text( disp, curX, curY, "Number of presses:" );
    curY += ROWSPACE;
    graphics_draw_text( disp, curX, curY, "1" );
    coordRepY = curY;
    coordRepX[ 0 ] = curX - SELECTSPACE;
       
    curX += COLSPACE;
    graphics_draw_text( disp, curX, curY, "5" );
    coordRepX[ 1 ] = curX - SELECTSPACE;
    
    curX += COLSPACE;
    graphics_draw_text( disp, curX, curY, "10" );
    coordRepX[ 2 ] = curX - SELECTSPACE;
    
    curX += COLSPACE;
    graphics_draw_text( disp, curX, curY, "20" );
    coordRepX[ 3 ] = curX - SELECTSPACE;
    
    curX += COLSPACE;
    graphics_draw_text( disp, curX, curY, "30" );
    coordRepX[ 4 ] = curX - SELECTSPACE;
    
    curX = COORDXINIT;
    curY += 3*ROWSPACE;
    
    graphics_draw_text( disp, curX, curY, "Press START to begin." );
    
    // Check for a button press.
    controller_scan();
    struct controller_data cont = get_keys_pressed();
    // We only want to register down press events.
    unsigned int up = 0;
    unsigned int down = 0;
    unsigned int left = 0;
    unsigned int right = 0;
    
    if ( cont.c[0].up && !prevCont.c[0].up ) {
      up = 1;
    } else if ( cont.c[0].down && !prevCont.c[0].down ) {
      down = 1;
    } else if ( cont.c[0].left && !prevCont.c[0].left ) {
      left = 1;
    } else if ( cont.c[0].right && !prevCont.c[0].right ) {
      right = 1;
    }
    
    // Store the current one.
    prevCont = cont;
    
    // Handle presses.
    if ( curCat == poll ) {
      // Update location.
      if ( down )
        curCat = rep;
      else if ( left ) {
        switch ( curPoll ) {
          case FiveMS:
            curPoll = ZeroMS;
            break;
          case TenMS:
            curPoll = FiveMS;
            break;
          case TwentyMS:
            curPoll = TenMS;
            break;
          case ThirtyMS:
            curPoll = TwentyMS;
            break;
          default:
            break;
        }
      } else if ( right ) {
        switch ( curPoll ) {
          case ZeroMS:
            curPoll = FiveMS;
            break;
          case FiveMS:
            curPoll = TenMS;
            break;
          case TenMS:
            curPoll = TwentyMS;
            break;
          case TwentyMS:
            curPoll = ThirtyMS;
            break;
          default:
            break;
        }
      }
    } else {
      if ( up )
        curCat = poll;
      else if ( left ) {
        switch ( curRep ) {
          case rep5:
            curRep = rep1;
            break;
          case rep10:
            curRep = rep5;
            break;
          case rep20:
            curRep = rep10;
            break;
          case rep30:
            curRep = rep20;
            break;
          default:
            break;
        }
      } else if ( right ) {
        switch ( curRep ) {
          case rep1:
            curRep = rep5;
            break;
          case rep5:
            curRep = rep10;
            break;
          case rep10:
            curRep = rep20;
            break;
          case rep20:
            curRep = rep30;
            break;
          default:
            break;
        }
      }
    }
    
    // Display settings.
    if ( curCat == poll ) {
      graphics_draw_text( disp, coordRepX[ curRep ], coordRepY, ">" );
      graphics_set_color( selCol, blackCol );
      graphics_draw_text( disp, coordPollX[ curPoll ], coordPollY, ">" );
      graphics_set_color( whiteCol, blackCol );
    } else {
      graphics_draw_text( disp, coordPollX[ curPoll ], coordPollY, ">" );
      graphics_set_color( selCol, blackCol );
      graphics_draw_text( disp, coordRepX[ curRep ], coordRepY, ">" );
      graphics_set_color( whiteCol, blackCol );
    }
    
    display_show( disp );
    
    // Start.
    if ( cont.c[0].start ) {
      // Set up things.
      uint32_t nrTests;
      switch ( curRep ) {
        case rep1:
          nrTests = 1;
          break;
        case rep5:
          nrTests = 5;
          break;
        case rep10:
          nrTests = 10;
          break;
        case rep20:
          nrTests = 20;
          break;
        case rep30:
          nrTests = 30;
          break;
        default:
          nrTests = 10;
          break;
      }
      
      // Set up a timer (if we need to).
      timer_link_t* tim = 0;
      
      if ( curPoll != ZeroMS ) {
        switch ( curPoll ) {
          case FiveMS:
            tim = new_timer( TIMER_TICKS(5000), TF_CONTINUOUS, checkButton);
            break;
          case TenMS:
            tim = new_timer( TIMER_TICKS(10000), TF_CONTINUOUS, checkButton);
            break;
          case TwentyMS:
            tim = new_timer( TIMER_TICKS(20000), TF_CONTINUOUS, checkButton);
            break;
          case ThirtyMS:
            tim = new_timer( TIMER_TICKS(30000), TF_CONTINUOUS, checkButton);
            break;
          default:
            break;
        }
      }
      
      // Start the actual measurings.
      // Initialize randomness.
      srand( timer_ticks() );
      for ( uint32_t iter = 0; iter < nrTests; ++iter ) {
        // Get the event appearing time.
        unsigned int waitTime = ( rand() % ( MAXWAITMS - MINWAITMS ) ) 
          + MINWAITMS;
          
        unsigned long long startT = TICKTOMS( timer_ticks() );
        
        // Reset the event.
        butPressShow = 0;
        
        // And the butSucc
        butSucc = 0;
        
        // Store the starting time.
        unsigned long long startTick = -1;
        unsigned long long endTick;
        
        while ( 1 ) {
          // Display.
          display_context_t disp = 0;
          while( !( disp = display_lock() ) ) {};
    
          // Set colors.
          graphics_set_color( whiteCol, blackCol );
          graphics_fill_screen( disp, 0 );
          
          #ifdef DEBUG
          char d[ 50 ];
          sprintf( d, "get_ticks_ms: %llu", TICKTOMS( timer_ticks() ) );
          graphics_draw_text( disp, 0, 200, d );
          #endif
          
          // Info text.
          char c[ 30 ];
          sprintf( c, "Run %lu / %lu", iter + 1, nrTests );
          graphics_draw_text( disp, COORDXINIT, COORDYINIT, c );
          
          // Check if the event happened.
          if ( TICKTOMS( timer_ticks() ) - startT >= waitTime || butPressShow ) {
            graphics_draw_text( disp, 140, 120, "Press A" );
            if ( butPressShow == 0 ) {
              startTick = timer_ticks();
            }
            butPressShow = 1;
          }
          
          // Show display.
          display_show( disp );
          
          // If instant. polling was chosen, we do this here.
          if ( curPoll == ZeroMS ) {
            checkButton( 0 );
          }
          
          // Check if the button was pressed.
          if ( butSucc ) {
            endTick = timer_ticks();
            break;
          }
        }
        
        // Store the latest time.
        latTicks[ iter ] = endTick - startTick;
      }
      
      // Stop the timer.
      if ( tim ) {
        stop_timer( tim );
      }
      
      // Now calculate average, min/max and std. deriv.
      unsigned long long avg = 0;
      unsigned long long min = -1;
      unsigned long long max = 0;
      
      for ( unsigned int i = 0; i < nrTests; ++i ) {
        min = ( min > latTicks[ i ] ? latTicks[ i ] : min );
        max = ( max < latTicks[ i ] ? latTicks[ i ] : max );
        avg += latTicks[ i ];
      }
      
      avg /= nrTests;
      
      float stdDer = -1.0f;
      // Only calculate the std. deriv. for nrTests > 1.
      if ( nrTests > 1 ) {
        long long sum = 0;
        for ( unsigned int i = 0; i < nrTests; ++i ) {
          long long tmp = (long long) latTicks[ i ] - (long long) avg;
          tmp *= tmp;
          sum += tmp;
        }
        
        sum /= ( nrTests - 1 );
        stdDer = sqrt( sum );
      }
      
      // Print out results.
      while( 1 ) {
        display_context_t disp = 0;
        while( !( disp = display_lock() ) ) {};
        graphics_fill_screen( disp, 0 );
        
        unsigned int x = COORDXINIT;
        unsigned int y = COORDYINIT;
        
        char c[100];
        sprintf( c, "Min: %.3f ms", TICKTOMS_DOUBLE( min ) );
        graphics_draw_text( disp, x, y, c );
        y += ROWSPACE;
        sprintf( c, "Max: %.3f ms", TICKTOMS_DOUBLE( max ) );
        graphics_draw_text( disp, x, y, c );
        y += ROWSPACE;
        sprintf( c, "Avg: %.3f ms", TICKTOMS_DOUBLE( avg ) );
        graphics_draw_text( disp, x, y, c );
        
        if ( curRep != rep1 ) {
          y += ROWSPACE;
          sprintf( c, "Std. Der.: %.3f ms", TICKTOMS_DOUBLE( stdDer ) );
          graphics_draw_text( disp, x, y, c );
        }
        
        y += ROWSPACE;
        graphics_draw_text( disp, x, y, "...press B to exit." );
        display_show( disp );
        
        controller_scan();
        struct controller_data cont = get_keys_pressed();
        
        if ( cont.c[0].B ) {
          break;
        }
      }
    }
  }
}

int main() {
  // Initialize stuff.
  init_interrupts();
  display_init( RESOLUTION_320x240, DEPTH_32_BPP, 2, GAMMA_NONE, 
    ANTIALIAS_RESAMPLE );
  timer_init();
  controller_init();
  
  // Init colors.
  blackCol = 0;
  whiteCol = graphics_make_color( 255, 255, 255, 255 );
  selCol = graphics_make_color( 0, 100, 200, 255 );
  
  // Go to the main menu.
  mainMenu();
  
  return 0;
}
