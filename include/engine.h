#ifndef MIROSLAVBEL_TETRIS_ENGINE_ENGINE_H
#define MIROSLAVBEL_TETRIS_ENGINE_ENGINE_H

#include <stdint.h>

/*! \mainpage tetris-engine
 *
 * Небольшой движок для тетриса.
 * 
 * Зависит от стандартной библиотеки Си. Не атомарен. Не потокобезопасен.
 * 
 * Имеет некоторые ограничения, однако позволяет пользователю самому
 * генерировать новые тетрамино и управлять вознаграждением за очищенные линии,
 * путем передачи функций за это ответственных в #initGame.
 * 
 * Пользователю необходимо самому вызывать функцию #tick для падения тетрамино.
 * 
 * Функции, представленные в tetris-engine можно поделить на следующие группы:
 *   - Создание и деалокация игры
 *     - #initGame
 *     - #startGame
 *     - #freeGame
 *   - Обработка ввода пользователя
 *     - #moveLeft
 *     - #moveRight
 *     - #rotateClockwise
 *     - #rotateAgainstClockwise
 *   - Тик
 *     - #tick
 *   - Доступ к игровому стакану
 *     - #getGameFieldPixel
 * 
 * Для доступа к игровому стакану (#Game::gameField) можно использовать макрос
 * #flatArrayAs2D или memory-safe функцию #getGameFieldPixel. Для доступа к
 * следующему тетрамино
 * \link #NextTetromino::pixels Game::nextTetromino::pixels\endlink можно
 * использовать только макрос #flatArrayAs2D.
 * 
 * \note Так как #TetrominoPixel не является атомарным, в многониточной
 * программе при доступе к игровому стакану и следующему тетрамино нужно
 * гарантировать, что никакая из функций не работает с игрой в данный момент
 * времени.
 * 
 * \note Данные в игровом стакане и следующем тетрамино являются
 * консистентными между вызовами функций.
 * 
 * \warning Какая-либо запись данных пользователем в #Game не предполагается.
 * Все действия записи в #Game должны выполнятся через определенные в
 * tetris-engine функции.
 */

/*!
 * \example ..\examples\gdi_example.c
 * \brief Пример для Windows GDI. Предполагает использования только одного
 * треда.
 */

/*!
 * \brief Позволяет обращаться к одномерному массиву как к двухмерному массиву
 * по координатам \a x и \a y.
 * \param[in,out] array одномерный массив
 * \param[in] x x-координата
 * \param[in] y y-координата
 * \param[in] width ширина двухмерного массива
 * \return значение из массива
 */
#define flatArrayAs2D(array, x, y, width) array[(y) * (width) + (x)]

/*!
 * \brief Ширина и высота тетрамино.
 *
 * Служит шириной и высотой для представления одномерных массивов
 * #ActiveTetromino::pixels и #NextTetromino::pixels как двухмерных массивов.
 * 
 * \see #tetrominoArrayMaxSize
 */
#define tetrominoMaxSize 4

/*!
 * \brief Длина одномерных массивов #ActiveTetromino::pixels и
 #NextTetromino::pixels.
 *
 * Равна \f$tetrominoMaxSize^2\f$.
 * 
 * \see #tetrominoMaxSize
 * \see #TetrominoPixelArray
 */
#define tetrominoArrayMaxSize 16

/*!
 * \brief Пиксел тетрамино.
 * 
 * Значение \a 0 означает, что пиксел свободен. Любое другое значение означает,
 * что пиксел занят. Разные ненулевые значения можно использовать, например,
 * для кодирования цветов тетрамино.
 */
typedef uint8_t TetrominoPixel;

/*!
 * \brief Массив пикселов тетрамино.
 * 
 * \see #TetrominoPixel
 */
typedef TetrominoPixel* TetrominoPixelArray;

/*!
 * \brief Вычисляет количество очков, которое игрок заработал за заполнение 
 * строк.
 * 
 * \param[in] cleanedLines количество заполненных за один тик строк. Может быть
 * \a 0
 * 
 * \return количество очков, которое игрок заработал за заполнение строк в
 * количестве \a cleanedLines
 */
typedef uint32_t GetScoreAddendFunction(int8_t cleanedLines);

/*!
 * \brief Следующие тетрамино.
 */
typedef struct tagNextTetromino {
   /*!
    * \brief Ширина и высота области, которую занимает тетрамино.
    * 
    * Может принимать значения [2 .. #tetrominoMaxSize].
    * 
    * Например, I тетрамино имеет #size равный #tetrominoMaxSize. O тетрамино
    * равный \a 2 . L- и S-образные равный \a 3 .
    */
   int8_t size;
   /*!
    * \brief Одномерный массив пикселов тетрамино.
    *
    * Тетрамино смещено к началу координат (координаты \a 0:0 ).
    * 
    * \note Длина одномерного массива равна #tetrominoArrayMaxSize. Для доступа
    * как к двухмерному массиву используйте макрос #flatArrayAs2D. Ширина и
    * высота двухмерного массива равна #tetrominoMaxSize.
    */
   TetrominoPixelArray pixels;
} NextTetromino;

/*!
 * \brief Генерирует следующее тетрамино.
 * 
 * #NextTetromino::size может принимать значения [2 .. #tetrominoMaxSize]. А
 * тетрамино в #NextTetromino::pixels должно быть смещено к началу координат
 * (координаты 0:0).
 * 
 * Функция обязана обработать каждый пиксел в #NextTetromino::pixels даже если
 * у сгенерированного тетрамино #NextTetromino::size меньше, чем
 * #tetrominoMaxSize.
 * 
 * \param[out] nextTetromino структура, в которую функция должна записать
 * cгенерированное тетрамино
 */
typedef void GetNextTetrominoFunction(NextTetromino* nextTetromino);

/*!
 * \brief Активное тетрамино.
 */
typedef struct tagActiveTetromino {
   /*!
    * \brief Ширина и высота области, которую занимает тетрамино.
    * 
    * Может принимать значения [2 .. #tetrominoMaxSize].
    * 
    * Например, I тетрамино имеет #size равный #tetrominoMaxSize. O тетрамино
    * равный \a 2 . L- и S-образные равный \a 3 .
    */
   int8_t size;
   /*!
    * \brief Смещение начала координат массива #pixels по оси \a x относительно
    * начала координат игрового стакана.
    * 
    * \warning Может быть отрицательным.
    */
   int8_t x;
   /*!
    * \brief Смещение начала координат массива #pixels по оси \a y относительно
    * начала координат игрового стакана.
    * 
    * \warning Может быть отрицательным.
    */
   int8_t y;
   /*!
    * \brief Одномерный массив пикселов тетрамино.
    *
    * Тетрамино смещено к началу координат (координаты \a 0:0 ).
    * 
    * \note Длина одномерного массива равна #tetrominoArrayMaxSize. Для доступа
    * как к двухмерному массиву используйте макрос #flatArrayAs2D. Ширина и
    * высота двухмерного массива равна #tetrominoMaxSize.
    */
   TetrominoPixelArray pixels;
} ActiveTetromino;

/*!
 * \brief Состояния игры.
 */
typedef enum tagGameStatus {
   /*!
    * \brief Игра создана, но ещё не было ни одного тика.
    * 
    * \warning При этом статусе #Game::nextTetromino и #Game::activeTetromino
    * аллоцированы, но не инициализированы.
    */
   initGameStatus,
   playGameStatus, ///< Игра идет.
   /*!
    * Игра закончена. Игрок выиграл, так как достигнут максимум очков.
    */
   endMaxScoreStatus,
   /*!
    * Игра закончена. Игрок проиграл, так как активному тетрамино некуда
    * упасть.
    */
   endPlayerLoose,
} GameStatus;

/*!
 * \brief Сама игра.
 */
typedef struct tagGame {
   /*!
    * \brief Состояние игры.
    */
   GameStatus status;
   /*!
    * \brief Ширина игрового стакана в пикселах.
    * 
    * Больше или равна #tetrominoMaxSize.
    */
   const int8_t width;
   /*!
    * \brief Высота игрового стакана в пикселах.
    * 
    * Больше или равна #tetrominoMaxSize.
    */
   const int8_t height;
   /*!
    * \brief Количество очков.
    * 
    * Меньше или равно #maxScore.
    */
   uint32_t score;
   /*!
    * \brief Максимально возможное количество очков.
    * 
    * Если игрок наберет больше очков, то значение #score установится в
    * #maxScore.
    */
   const uint32_t maxScore;
   /*!
    * \brief Одномерный массив пикселов тетрамино игрового стакана.
    * 
    * \note Длина одномерного массива равна #width * #height. Для доступа
    * как к двухмерному массиву используйте макрос #flatArrayAs2D. Ширина и
    * высота двухмерного массива равна #width и #height соответственно.
    */
   TetrominoPixelArray const gameField;
   /*!
    * \brief Активное тетрамино.
    * 
    * \warning Если значение #status установлено в #initGameStatus, активное
    * тетрамино аллоцировано, но не инициализировано.
    */
   ActiveTetromino* activeTetromino;
   /*!
    * \brief Следующего тетрамино.
    */
   NextTetromino* nextTetromino;
   /*!
    * \brief Функция, генерирующая следующее тетрамино.
    */
   GetNextTetrominoFunction* const getNextTetromino;
   /*!
    * \brief Функция, вычисляющая количество очков, которое игрок заработал за
    * заполнение строк.
    */
   GetScoreAddendFunction* const getScoreAddend;
} Game;

/*!
 * \brief Инициализирует игру.
 * 
 * В #Game::status записывает #initGameStatus. 
 * 
 * \warning Функция аллоцирует массивы для следующего и активного тетрамино, но
 * не инициализирует их. Для полной инициализации требуется вызвать функцию
 * #startGame.
 * 
 * \param[in] width ширина игрового стакана. Должна быть больше или равна 
 * #tetrominoMaxSize
 * \param[in] height высота игрового стакана. Должна быть больше или равна 
 * #tetrominoMaxSize
 * \param[in] maxScore максимально возможное количество очков
 * \param[in] getNextTetrominoFunction функция, генерирующая следующее
 * тетрамино
 * \param[in] getScoreAddendFunction функция, вычисляющая количество очков,
 * которое игрок заработал за заполнение строк
 * 
 * \return
 *          - 1) \a NULL в случае ошибки (нехватка памяти);
 *          - 2) указатель на структуру.
 * 
 * \see #startGame
 * \see #Game::maxScore
 */
Game* initGame(int8_t width, int8_t height, int32_t maxScore,
      GetNextTetrominoFunction* const getNextTetrominoFunction, 
      GetScoreAddendFunction* const getScoreAddendFunction);

/*!
 * \brief Запускает игру.
 * 
 * \param[in,out] game игра, где в #Game::status установлено значение
 * #initGameStatus
 */
void startGame(Game* game);

/*!
 * \brief Возвращает значение пиксела из игрового стакана.
 * 
 * \note Если пиксел находится во вне игрового стакана, то
 * - если пиксел находится прямо сверху игрового поля (но не по диагонали), то
 * вернет \a 0;
 * - если пиксел находится в любом другом месте во вне игрового стакана, то
 * вернет \a 1.
 * 
 * \param[in] game игра
 * \param[in] x x-координата пиксела
 * \param[in] y y-координата пиксела
 * 
 * \return значение заданного пиксела
 */
TetrominoPixel getGameFieldPixel(Game* game, int8_t x, int8_t y);

/*!
 * \brief Если это возможно, поворачивает активное тетрамино по часовой
 * стрелке.
 * 
 * \param[in,out] game игра, где в #Game::status установлено значение
 * #playGameStatus
 * 
 * \return
 *          - 1) \a 0 если тетрамино удалось повернуть по часовой стрелке
 *          - 2) \a 1 если тетрамино не удалось повернуть по часовой стрелке
 */
unsigned rotateClockwise(Game* game);

/*!
 * \brief Если это возможно, поворачивает активное тетрамино против часовой
 * стрелки.
 * 
 * \param[in,out] game игра, где в #Game::status установлено значение
 * #playGameStatus
 * 
 * \return
 *          - 1) \a 0 если тетрамино удалось повернуть против часовой стрелке
 *          - 2) \a 1 если тетрамино не удалось повернуть против часовой
 * стрелке
 */
unsigned rotateAgainstClockwise(Game* game);

/*!
 * \brief Если это возможно, двигает активное тетрамино влево.
 * 
 * \param[in,out] game игра, где в #Game::status установлено значение
 * #playGameStatus
 * 
 * \return
 *          - 1) \a 0 если активное тетрамино удалось подвинуть влево
 *          - 2) \a 1 если активное тетрамино не удалось подвинуть влево
 */
unsigned moveLeft(Game* game);

/*!
 * \brief Если это возможно, двигает активное тетрамино вправо.
 * 
 * \param[in,out] game игра, где в #Game::status установлено значение
 * #playGameStatus
 * 
 * \return
 *          - 1) \a 0 если активное тетрамино удалось подвинуть вправо
 *          - 2) \a 1 если активное тетрамино не удалось подвинуть вправо
 */
unsigned moveRight(Game* game);

/*!
 * \brief Имплементриует тик. 
 * 
 * Если достигнут максимум очков или игрок проиграл выставляет соответствующие 
 * статусы в #Game::status.
 * 
 * \param[in,out] game игра, где в #Game::status установлено значение
 * #playGameStatus
 * 
 * \return
 *          - 1) \a 0 если активное тетрамино удалось подвинуть вниз
 *          - 2) \a 1 если активное тетрамино не удалось подвинуть вниз
 *          - 3) \a 2 если достигнут максимум очков
 *          - 4) \a 3 если игрок проиграл
 */
unsigned tick(Game* game);

/*!
 * \brief Освобождает игру.
 * 
 * \param[out] game игра
 */
void freeGame(Game* game);

#endif
