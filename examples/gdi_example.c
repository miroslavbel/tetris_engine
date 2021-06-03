// Example for one thread.

#include <Windows.h> // example use windows-style message quenue

#include <engine.h>

GetNextTetrominoFunction *const ourGetNextTetrominoFunction;
GetScoreAddendFunction *const ourGetScoreAddendFunction;

// For simplicity of example we allow play only one time
// In this way before game creation it can be null. And
// after game creation it's always a valid ptr (we don't
// need atomic because now it is immutable).
Game* game = NULL;


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void handler_paint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void handler_keyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void handler_timer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main() {
   // GetModuleHandleW in a real app will be also needed or use cl compiler with WinMain-like entry point

   // allocate memory
   game = initGame(10, 20, 1000, ourGetNextTetrominoFunction, ourGetScoreAddendFunction);
   if (!game)  { freeGame(game); return -1; } //< handle OutOfMemory error

   // create window with WNDCLASS::lpfnWndProc = WndProc
   // use RegisterClass, CreateWindow, ShowWindow, UpdateWindow
   HWND hWnd = NULL;

   // start game
   startGame(game);

   // SetTimer call
   SetTimer(hWnd, 1, 500, NULL);

   // message quenue routine
   MSG msg;
   BOOL bRet;
   while( (bRet = GetMessageW(&msg, NULL, 0, 0)) != 0) {
      if (bRet == -1) {
         // handle the error and possibly exit
      } else {
         TranslateMessage(&msg); 
         DispatchMessageW(&msg); 
      }
   }
   freeGame(game); // do we really need it before call to ExitProcess?
   return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   switch (uMsg) {
      case WM_PAINT:
         handler_paint(hWnd, uMsg, wParam, lParam);
         return 0; // an application should return zero if it processes WM_PAINT message
      case WM_KEYDOWN:
         handler_keyDown(hWnd, uMsg, wParam, lParam);
         return 0; // an application should return zero if it processes WM_KEYDOWN message
      case WM_TIMER:
         mainWindowHandler_timer(wParam);
         return 0; // an application should return zero if it processes WM_TIMER message
      case WM_CLOSE:
        DestroyWindow(hWnd);
        PostQuitMessage(0);
        return 0; // an application should return zero if it processes WM_DESTROY message
      default:
         return DefWindowProcW(hWnd, uMsg, wParam, lParam);
   }
}

void handler_paint(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   for (int8_t y = game->height - 1; y >= 0; --y) {
      int8_t realY = game->height - 1 - y;
      for (int8_t x = 0; x < game->width; ++x) {
         TetrominoPixel tetrominoPixel = flatArrayAs2D(game->gameField, x, y, game->width);
         // if pixel not empty (not 0)
         if (tetrominoPixel) {
            // then draw
            // checkout, for example, https://docs.microsoft.com/en-us/windows/win32/gdi/rectangles

            // remember we can use different TetrominoPixel values as colors

         }
      }
   }
   // draw the same way game->nextTetromino

   // draw game->score
   // checkout, for example, https://docs.microsoft.com/en-us/windows/win32/gdi/fonts-and-text

}

void handler_keyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   unsigned isGameFieldChange;
   // check game->status
   if (game->status == playGameStatus) {
      switch (wParam) {
         case VK_LEFT:
         case 0x41: // A
            isGameFieldChange = moveLeft(game);
            break;
         case VK_RIGHT:
         case 0x44: // D
            isGameFieldChange = moveRight(game);
            break;
         case 0x51: // Q
            isGameFieldChange = rotateAgainstClockwise(game);
            break;
         case 0x45: // E
            isGameFieldChange = rotateClockwise(game);
            break;
      }
      if (!isGameFieldChange) {
         // send WM_PAINT
         InvalidateRect(hWnd, NULL, FALSE);
      }
   }
}

void handler_timer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
   unsigned status = tick(game);
   if (status == 2 || status == 3) {
      // game ends then kill the timer
   } else {
      // success or fail (but we need to update screen because of 
      //   nextTetromino move to activeTetramino and because
      //   the new nextTetromino was generated)
      InvalidateRect(hWnd, NULL, FALSE);
   }
}
