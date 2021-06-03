#include <string.h> // for memcpy, memmove, memset
#include <stdlib.h> // for calloc, free, malloc,
#include <math.h>   // for fabs, round

#include <engine.h>

Game* initGame(int8_t width, int8_t height, int32_t maxScore,
      GetNextTetrominoFunction* const getNextTetrominoFunction, 
      GetScoreAddendFunction* const getScoreAddendFunction) {
   Game* game = (Game*) malloc(sizeof(Game));
   ActiveTetromino* activeTetromino = (ActiveTetromino*) malloc(sizeof(ActiveTetromino));
   NextTetromino* nextTetromino = (NextTetromino*) malloc(sizeof(NextTetromino));
   TetrominoPixelArray gameField = (TetrominoPixelArray) calloc(width * height, sizeof(TetrominoPixel));
   TetrominoPixelArray firstTetrominoArray = (TetrominoPixelArray) malloc(tetrominoMaxSize * tetrominoMaxSize * sizeof(uint8_t));
   TetrominoPixelArray secondTetrominoArray = (TetrominoPixelArray) malloc(tetrominoMaxSize * tetrominoMaxSize * sizeof(uint8_t));
   if (!(game && activeTetromino && nextTetromino && gameField && firstTetrominoArray && secondTetrominoArray)) {
      free(game);
      free(activeTetromino);
      free(nextTetromino);
      free(gameField);
      free(firstTetrominoArray);
      free(secondTetrominoArray);
      return NULL;
   }
   game->status = initGameStatus;
   *(int8_t*) &game->width = width;
   *(int8_t*) &game->height = height;
   game->score = 0;
   *(uint32_t*) &game->maxScore = maxScore;
   *(TetrominoPixelArray*) &game->gameField = gameField;
   game->activeTetromino = activeTetromino;
   game->activeTetromino->pixels = firstTetrominoArray;
   game->nextTetromino = nextTetromino;
   game->nextTetromino->pixels = secondTetrominoArray;
   *(GetNextTetrominoFunction**) &game->getNextTetromino = getNextTetrominoFunction;
   *(GetScoreAddendFunction**) &game->getScoreAddend = getScoreAddendFunction;
   return game;
}

void startGame(Game* game) {
   TetrominoPixelArray temp = game->activeTetromino->pixels;
   game->getNextTetromino(game->nextTetromino);
   game->activeTetromino->pixels = game->nextTetromino->pixels;
   game->activeTetromino->size = game->nextTetromino->size;
   game->activeTetromino->x = game->width / 2 - game->activeTetromino->size / 2;
   game->activeTetromino->y = game->height;
   game->nextTetromino->pixels = temp;
   game->getNextTetromino(game->nextTetromino);
   game->status = playGameStatus;
}

TetrominoPixel getGameFieldPixel(Game* game, int8_t x, int8_t y) {
   if (x < 0 || x >= game->width) {
      return 1;
   }
   if (y < 0) {
      return 1;
   }
   if (y >= game->height) {
      return 0;
   }
   return flatArrayAs2D(game->gameField, x, y, game->width);
}

/*!
 * \brief Устанавливает пиксел в игровом стакане.
 * 
 * \note Если пиксел находится вовне игрового стакана, то ничего не установит.
 * 
 * \param[in,out] game указатель на структуру
 * \param[in] pixel пиксел
 * \param[in] x x-координата пиксела
 * \param[in] y y-координата пиксела
 */
void setPixel(Game* game, TetrominoPixel pixel, int8_t x, int8_t y) {
   if (x < 0 || x >= game->width) {
      return;
   }
   if (y < 0) {
      return;
   }
   if (y >= game->height) {
      return;
   }
   flatArrayAs2D(game->gameField, x, y, game->width) = pixel;
}

/*!
 * \brief Может ли активное тетрамино подвинутся вниз?
 * 
 * \warning В игровом стакане должна отсутствовать информация об активном
 * тетрамино.
 * 
 * \param[in] game указатель на структуру
 * 
 * \return  
 *          - 1) \a 0 если не может
 *          - 2) \a 1 если может
 */
unsigned canActiveTetrominoMoveDown(Game* game) {
   for (int y = 0; y < tetrominoMaxSize; ++y) {
      for (int x = 0; x < tetrominoMaxSize; ++x) {
         if(flatArrayAs2D(game->activeTetromino->pixels, x, y, tetrominoMaxSize)) {
            if(getGameFieldPixel(game, game->activeTetromino->x + x, game->activeTetromino->y + y - 1)) {
               return 0;
            }
         }
      }
   }
   return 1;
}

/*!
 * \brief Может ли активное тетрамино повернутся по часовой стрелке?
 * 
 * \warning В игровом стакане должна отсутствовать информация об активном
 * тетрамино.
 * 
 * \param[in] game указатель на структуру
 * 
 * \return  
 *          - 1) \a 0 если не может
 *          - 2) \a 1 если может
 */
unsigned canActiveTetrominoRotateClockwise(Game* game) {
   double tetrominoCenter = ((double) (game->activeTetromino->size - 1)) / 2.;
   for (int sourceY = 0; sourceY < game->activeTetromino->size; ++sourceY) {
      for (int sourceX = 0; sourceX < game->activeTetromino->size; ++sourceX) {
         if(flatArrayAs2D(game->activeTetromino->pixels, sourceX, sourceY, tetrominoMaxSize)) {
            double relativeSourceX = (double) sourceX - tetrominoCenter;
            double relativeSourceY = (double) sourceY - tetrominoCenter;
            double relativeTargetX = relativeSourceY;
            double relativeTargetY = -relativeSourceX;
            int targetX = (int) round(relativeTargetX + tetrominoCenter);
            int targetY = (int) round(relativeTargetY + tetrominoCenter);
            if (relativeSourceX > relativeSourceY) {
               // низ право
               if (fabs(relativeSourceX) > fabs(relativeSourceY)) {
                  // право
                  // -y, -x
                  for (int checkY = sourceY; checkY >= targetY; --checkY) {
                     if(getGameFieldPixel(game, sourceX + game->activeTetromino->x, checkY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
                  for (int checkX = sourceX; checkX >= targetX; --checkX) {
                     if(getGameFieldPixel(game, checkX + game->activeTetromino->x, targetY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
               } else {
                  // низ
                  // -x, +y
                  for (int checkX = sourceX; checkX >= targetX; --checkX) {
                     if(getGameFieldPixel(game, checkX + game->activeTetromino->x, sourceY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
                  for (int checkY = sourceY; checkY <= targetY; ++checkY) {
                     if(getGameFieldPixel(game, targetX + game->activeTetromino->x, checkY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
               }
            } else {
               // верх лево
               if (fabs(relativeSourceX) > fabs(relativeSourceY)) {
                  // лево
                  // +y, +x
                  for (int checkY = sourceY; checkY <= targetY; ++checkY) {
                     if(getGameFieldPixel(game, sourceX + game->activeTetromino->x, checkY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
                  for (int checkX = sourceX; checkX <= targetX; ++checkX) {
                     if(getGameFieldPixel(game, checkX + game->activeTetromino->x, targetY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
               } else {
                  // верх
                  // +x, -y
                  for (int checkX = sourceX; checkX <= targetX; ++checkX) {
                     if(getGameFieldPixel(game, checkX + game->activeTetromino->x, sourceY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
                  for (int checkY = sourceY; checkY >= targetY; --checkY) {
                     if(getGameFieldPixel(game, targetX + game->activeTetromino->x, checkY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
               }
            }
         }
      }
   }
   return 1;
}

/*!
 * \brief Может ли активное тетрамино повернутся против часовой стрелки?
 * 
 * \warning В игровом стакане должна отсутствовать информация об активном
 * тетрамино.
 * 
 * \param game указатель на структуру
 * 
 * \return  
 *          - 1) \a 0 если не может
 *          - 2) \a 1 если может
 */
unsigned canActiveTetrominoRotateAgainstClockwise(Game* game) {
   double tetrominoCenter = ((double) (game->activeTetromino->size - 1)) / 2.;
   for (int sourceY = 0; sourceY < game->activeTetromino->size; ++sourceY) {
      for (int sourceX = 0; sourceX < game->activeTetromino->size; ++sourceX) {
         if(flatArrayAs2D(game->activeTetromino->pixels, sourceX, sourceY, tetrominoMaxSize)) {
            double relativeSourceX = (double) sourceX - tetrominoCenter;
            double relativeSourceY = (double) sourceY - tetrominoCenter;
            double relativeTargetX = -relativeSourceY;
            double relativeTargetY = relativeSourceX;
            int targetX = (int) round(relativeTargetX + tetrominoCenter);
            int targetY = (int) round(relativeTargetY + tetrominoCenter);
            if (relativeSourceX > relativeSourceY) {
               // низ право
               if (fabs(relativeSourceX) > fabs(relativeSourceY)) {
                  // право
                  // +y, -x
                  for (int checkY = sourceY; checkY <= targetY; ++checkY) {
                     if(getGameFieldPixel(game, sourceX + game->activeTetromino->x, checkY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
                  for (int checkX = sourceX; checkX >= targetX; --checkX) {
                     if(getGameFieldPixel(game, checkX + game->activeTetromino->x, targetY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
               } else {
                  // низ
                  // +x, +y
                  for (int checkX = sourceX; checkX <= targetX; ++checkX) {
                     if(getGameFieldPixel(game, checkX + game->activeTetromino->x, sourceY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
                  for (int checkY = sourceY; checkY <= targetY; ++checkY) {
                     if(getGameFieldPixel(game, targetX + game->activeTetromino->x, checkY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
               }
            } else {
               // верх лево
               if (fabs(relativeSourceX) > fabs(relativeSourceY)) {
                  // лево
                  // -y, +x
                  for (int checkY = sourceY; checkY >= targetY; --checkY) {
                     if(getGameFieldPixel(game, sourceX + game->activeTetromino->x, checkY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
                  for (int checkX = sourceX; checkX <= targetX; ++checkX) {
                     if(getGameFieldPixel(game, checkX + game->activeTetromino->x, targetY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
               } else {
                  // верх
                  // -x, -y
                  for (int checkX = sourceX; checkX >= targetX; --checkX) {
                     if(getGameFieldPixel(game, checkX + game->activeTetromino->x, sourceY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
                  for (int checkY = sourceY; checkY >= targetY; --checkY) {
                     if(getGameFieldPixel(game, targetX + game->activeTetromino->x, checkY + game->activeTetromino->y)) {
                        return 0;
                     }
                  }
               }
            }
         }
      }
   }
   return 1;
}

/*!
 * \brief Заносит информацию об активном тетрамино в игровой стакан.
 * 
 * \warning Каких-либо проверок не производит.
 * 
 * \param[in,out] game указатель на структуру
 */
void pushActiveTetrominoInfo(Game* game) {
   for (int y = 0; y < game->activeTetromino->size; ++y) {
      for (int x = 0; x < game->activeTetromino->size; ++x) {
         TetrominoPixel tetrominoPixel = flatArrayAs2D(game->activeTetromino->pixels, x, y, tetrominoMaxSize);
         if (tetrominoPixel) {
            setPixel(game, tetrominoPixel, game->activeTetromino->x + x, game->activeTetromino->y + y);
         }
      }
   }
}

/*!
 * \brief Удаляет информацию об активном тетрамино из игрового стакана.
 * 
 * \warning Каких-либо проверок не производит.
 * 
 * \param[in,out] game указатель на структуру
 */
void popActiveTetrominoInfo(Game* game) {
   for (int y = 0; y < game->activeTetromino->size; ++y) {
      for (int x = 0; x < game->activeTetromino->size; ++x) {
         if (flatArrayAs2D(game->activeTetromino->pixels, x, y, tetrominoMaxSize)) {
            setPixel(game, 0, game->activeTetromino->x + x, game->activeTetromino->y + y);
         }
      }
   }
}

/*!
 * \brief Если это возможно, двигает активное тетрамино вниз на одну строчку.
 *
 * \note На момент вызова функции информация об активном тетрамино должна быть
 * в игровом стакане. После работы функции и при удаче, и при неудаче
 * информация об активном тетрамино будет в игровом стакане.
 * 
 * \param[in,out] game указатель на структуру
 * 
 * \return  
 *          - 1) \a 0 если не может
 *          - 2) \a 1 если может
 */
unsigned moveActiveTetrominoDown(Game* game) {
   popActiveTetrominoInfo(game);
   if (canActiveTetrominoMoveDown(game)) {
      --game->activeTetromino->y;
      pushActiveTetrominoInfo(game);
      return 1;
   } else {
      pushActiveTetrominoInfo(game);
      return 0;
   }
}

unsigned rotateClockwise(Game* game) {
   popActiveTetrominoInfo(game);
   if (canActiveTetrominoRotateClockwise(game)) {
      TetrominoPixel tempTetrominoBuffer[tetrominoArrayMaxSize] = {0};
      double tetrominoCenter = ((double) (game->activeTetromino->size - 1)) / 2.;
      for (int sourceY = 0; sourceY < game->activeTetromino->size; ++sourceY) {
         for (int sourceX = 0; sourceX < game->activeTetromino->size; ++sourceX) {
            TetrominoPixel tetrominoPixel = flatArrayAs2D(game->activeTetromino->pixels, sourceX, sourceY, tetrominoMaxSize);
            if(tetrominoPixel) {
               double relativeSourceX = (double) sourceX - tetrominoCenter;
               double relativeSourceY = (double) sourceY - tetrominoCenter;
               double relativeTargetX = relativeSourceY;
               double relativeTargetY = -relativeSourceX;
               int targetX = (int) round(relativeTargetX + tetrominoCenter);
               int targetY = (int) round(relativeTargetY + tetrominoCenter);
               flatArrayAs2D(tempTetrominoBuffer, targetX, targetY, tetrominoMaxSize) = tetrominoPixel;
            }
         }
      }
      memcpy(game->activeTetromino->pixels, &tempTetrominoBuffer, tetrominoArrayMaxSize);
      pushActiveTetrominoInfo(game);
      return 0;
   } else {
      pushActiveTetrominoInfo(game);
      return 1;
   }
}

unsigned rotateAgainstClockwise(Game* game) {
   popActiveTetrominoInfo(game);
   if (canActiveTetrominoRotateAgainstClockwise(game)) {
      TetrominoPixel tempTetrominoBuffer[tetrominoArrayMaxSize] = {0};
      double tetrominoCenter = ((double) (game->activeTetromino->size - 1)) / 2.;
      for (int sourceY = 0; sourceY < game->activeTetromino->size; ++sourceY) {
         for (int sourceX = 0; sourceX < game->activeTetromino->size; ++sourceX) {
            TetrominoPixel tetrominoPixel = flatArrayAs2D(game->activeTetromino->pixels, sourceX, sourceY, tetrominoMaxSize);
            if(tetrominoPixel) {
               double relativeSourceX = (double) sourceX - tetrominoCenter;
               double relativeSourceY = (double) sourceY - tetrominoCenter;
               double relativeTargetX = -relativeSourceY;
               double relativeTargetY = relativeSourceX;
               int targetX = (int) round(relativeTargetX + tetrominoCenter);
               int targetY = (int) round(relativeTargetY + tetrominoCenter);
               flatArrayAs2D(tempTetrominoBuffer, targetX, targetY, tetrominoMaxSize) = tetrominoPixel;
            }
         }
      }
      memcpy(game->activeTetromino->pixels, &tempTetrominoBuffer, tetrominoArrayMaxSize);
      pushActiveTetrominoInfo(game);
      return 0;
   } else {
      pushActiveTetrominoInfo(game);
      return 1;
   }
}

/*!
 * \brief Очищает полностью занятые строки.
 * 
 * \param game[in,out] указатель на структуру
 * 
 * \return кол-во очищенный строк
 */
int8_t cleanLines(Game* game) {
   int8_t cleanedLines = 0;
   for (int8_t y = game->height - 1; y >= 0; --y) {
      unsigned isLineFull = 1;
      for (int8_t x = 0; x < game->width; ++x) {
         if (!getGameFieldPixel(game, x, y)) {
            isLineFull = 0;
            break;
         }
      }
      if (isLineFull) {
         // сдвинуть линии
         memmove(&flatArrayAs2D(game->gameField, 0, y, game->width), 
               &flatArrayAs2D(game->gameField, 0, y + 1, game->width),
               (game->height - y - 1) * game->width);
         // заполнить последную линию нулями
         memset(&flatArrayAs2D(game->gameField, 0, game->height - 1, game->width), 0, sizeof(TetrominoPixel) * game->width);
         ++cleanedLines;
      }
   }
   return cleanedLines;
}

/*!
 * \brief Находится ли активное тетрамино полностью в игровом стакане.
 * 
 * Проверяет только на нахождение прямо сверху от игрового стакана.
 * 
 * \param game[in] указатель на структуру
 * 
 * \return  
 *          - 1) \a 0 если как минимум часть тетрамино находится во вне 
 * игрового стакана
 *          - 2) \a 1 если тетрамино полностью находится в игровом стакане
 */
unsigned isActiveTetrominoInGameBoard(Game* game) {
   for (int8_t y = game->activeTetromino->size - 1; y >= 0; --y) {
      for (int8_t x = 0; x < game->activeTetromino->size; ++x) {
         if (flatArrayAs2D(game->activeTetromino->pixels, x, y, tetrominoMaxSize)) {
            if (game->activeTetromino->y + y >= game->height) {
               return 0;
            }
         }
      }
   }
   return 1;
}

unsigned tick(Game* game) {;
   if (!moveActiveTetrominoDown(game)) {
      if(!isActiveTetrominoInGameBoard(game)) {
         game->status = endPlayerLoose;
         return 3;
      } else {
         int8_t cleanedLines = cleanLines(game);
         uint32_t scoredAddend = game->getScoreAddend(cleanedLines);
         uint32_t scoreCopy = game->score;
         scoreCopy += scoredAddend;
         if (scoreCopy < game->score || scoreCopy > game->maxScore) {
            game->score = game->maxScore;
            game->status = endMaxScoreStatus;
            return 2;
         }
         game->score = scoreCopy;
         // nextTetromino -> activeTetromino, init nextTetromino
         TetrominoPixelArray activeTetrominoArray = game->activeTetromino->pixels;
         game->activeTetromino->pixels = game->nextTetromino->pixels;
         game->activeTetromino->size = game->nextTetromino->size;
         game->activeTetromino->x = game->width / 2 - tetrominoMaxSize / 2;
         game->activeTetromino->y = game->height;
         game->nextTetromino->pixels = activeTetrominoArray;
         game->getNextTetromino(game->nextTetromino);
         return 1;
      }
   }
   return 0;
}

/*!
 * \brief Двигает тетрамино влево.
 * 
 * \param[in,out] game указатель на структуру
 * 
 * \return  
 *          - 1) \a 0 если тетрамино удалось подвинуть влево
 *          - 2) \a 1 если тетрамино не удалось подвинуть влево
 */
unsigned moveLeft(Game* game) {
   popActiveTetrominoInfo(game);
   for (int y = 0; y < game->activeTetromino->size; ++y) {
      for (int x = 0; x < game->activeTetromino->size; ++x) {
         if(flatArrayAs2D(game->activeTetromino->pixels, x, y, tetrominoMaxSize)) {
            if(getGameFieldPixel(game, game->activeTetromino->x + x - 1, game->activeTetromino->y + y)) {
               pushActiveTetrominoInfo(game);
               return 1;
            }
         }
      }
   }
   --game->activeTetromino->x;
   pushActiveTetrominoInfo(game);
   return 0;
}

/*!
 * \brief Двигает тетрамино вправо.
 * 
 * \param[in,out] game указатель на структуру
 * 
 * \return
 *          - 1) \a 0 если тетрамино удалось подвинуть вправо
 *          - 2) \a 1 если тетрамино не удалось подвинуть вправо
 */
unsigned moveRight(Game* game) {
   popActiveTetrominoInfo(game);
   for (int y = 0; y < game->activeTetromino->size; ++y) {
      for (int x = 0; x < game->activeTetromino->size; ++x) {
         if(flatArrayAs2D(game->activeTetromino->pixels, x, y, tetrominoMaxSize)) {
            if(getGameFieldPixel(game, game->activeTetromino->x + x + 1, game->activeTetromino->y + y)) {
               pushActiveTetrominoInfo(game);
               return 1;
            }
         }
      }
   }
   ++game->activeTetromino->x;
   pushActiveTetrominoInfo(game);
   return 0;
}

void freeGame(Game* game) {
   if (game) {
      if (game->activeTetromino) {
         free(game->activeTetromino->pixels);
      }
      free(game->activeTetromino);
      if (game->nextTetromino) {
         free(game->nextTetromino->pixels);
      }
      free(game->nextTetromino);
      free(game->gameField);
   }
   free(game);
}
