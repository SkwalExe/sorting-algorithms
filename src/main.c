#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include "utils.h"

#define RED "\x1b[91m"
#define YELLOW "\x1b[93m"
#define PURPLE "\x1b[95m"
#define WHITE "\x1b[97m"
#define RED_BG "\x1b[41m"
#define YELLOW_BG "\x1b[43m"
#define PURPLE_BG "\x1b[45m"
#define WHITE_BG "\x1b[47m"

#define RESET "\x1b[0m"
#define RED_BLOCK RED_BG "  " RESET
#define YELLOW_BLOCK YELLOW_BG "  " RESET
#define PURPLE_BLOCK PURPLE_BG "  " RESET
#define WHITE_BLOCK WHITE_BG "  " RESET


#define VERSION "0.2.0"
#define TOP_SPACE 2

typedef int bar;
typedef struct barlist
{
  bar *bars;
  int selected;
  int len;
} barlist;

int width = 0, height = 0;

// 0 => all
// 1 => selection sort
// 2 => merge sort
// 3 => bubble sort
int algorithm = 0;

int step = 15;
int precision = 100;

void sigint_handler(int sig)
{
  if (sig == SIGINT)
  {
    print_at(0, height + TOP_SPACE + 2, "\n\nBye !\n");
    show_cursor();
    exit(0);
  }
}

void set_signal_action(void)
{
  struct sigaction act;

  memset(&act, 0, sizeof(act));
  act.sa_handler = &sigint_handler;
  sigaction(SIGINT, &act, NULL);
}



// Inverse bar at index a with bar at index b
void inverse(barlist *bars, int a, int b)
{
  bar temp = bars->bars[a];
  bars->bars[a] = bars->bars[b];
  bars->bars[b] = temp;
}

void show_bar(int bar_height, int prev_bar_height, int x, bool selected, bool prev_selected)
{
  // real height in the terminal
  int real_bar_height = bar_height * height / precision;
  int real_prev_bar_height = prev_bar_height * height / precision;

  // If the bar was not selected before
  if (!prev_selected)
  {
    // If the bar was raised and still isn't selected
    if (!selected && bar_height > prev_bar_height)
    {
      // Draw the top in white
      for (int y = height - real_bar_height + 1 + TOP_SPACE; y < height - real_prev_bar_height + 1 + TOP_SPACE; y++)
        print_at(x * 2, y, WHITE_BLOCK);
      return;
    }
    // if it was lowered
    else if (bar_height < prev_bar_height)
    {
      // Erase the top of the bar
      for (int y = height - real_prev_bar_height + 1 + TOP_SPACE; y < height - real_bar_height + 1 + TOP_SPACE; y++)
        print_at(x * 2, y, "  ");
    }

    // and redraw the bar in red if now selected
    if (selected)
    {
      for (int y = height - real_bar_height + 1 + TOP_SPACE; y < height + 1 + TOP_SPACE; y++)
        print_at(x * 2, y, RED_BLOCK);
    }
  }
  else 
  // If the bar was selected before
  { 
    // if it was raised
    // and is still selected
    if (selected && bar_height > prev_bar_height)
    { 
      // Draw the top in red
      // The bottom should already be in red
      for (int y = height - real_bar_height + 1 + TOP_SPACE; y < height - real_prev_bar_height + 1 + TOP_SPACE; y++)
        print_at(x * 2, y, RED_BLOCK);
    }
    // If the bar was lowered
    else if (bar_height < prev_bar_height)
    {
      // Erase the top of the bar
      for (int y = height - real_prev_bar_height + 1 + TOP_SPACE; y < height - real_bar_height + 1 + TOP_SPACE; y++)
        print_at(x * 2, y, "  ");
    }

    // if it is no longer selected
    // redraw the bar in white
    // if it is still selected
    // the bar should already be in red
    if (!selected)
    {
      for (int y = height - real_bar_height + 1 + TOP_SPACE; y < height + 1 + TOP_SPACE; y++)
        print_at(x * 2, y, WHITE_BLOCK);
    }

  }
}

void show(barlist *bars, barlist *prev_bars, char *text, int ms)
{
  print_at(0, 0, "");
  printf("\33[2K");
  print_at(0, 0, text);


  bool selection_changed = bars->selected != prev_bars->selected;


  for (int i = 0; i < bars->len; i++) 
  {
    // If the bar height has changed
    // or if the bar has been selected or deselected
    if (bars->bars[i] != prev_bars->bars[i] || (selection_changed && (bars->selected == i || prev_bars->selected == i)))
    {
      show_bar(bars->bars[i], prev_bars->bars[i], i, bars->selected == i, prev_bars->selected == i);
      prev_bars->bars[i] = bars->bars[i];        
    }
  }

  if (selection_changed)
    prev_bars->selected = bars->selected;


  fflush(stdout);
  sleep_ms(ms);
}
void shuffle(barlist *bars, barlist *prev_bars)
{
  for (int x = 0; x < bars->len; x++)
  {
    int swap_with = random_int(0, bars->len - 1);
    inverse(bars, x, swap_with);
    bars->selected = swap_with;
    show(bars, prev_bars, "shuffling", step);
  }
  bars->selected = -1;
  show(bars, prev_bars, "Preparing", 1000);
}

void highlight_all(barlist *bars, barlist *prev_bars)
{
  for (int i = 0; i < bars->len; i++)
  {
    bars->selected = i;
    show(bars, prev_bars, "sorted", step);
  }
  bars->selected = -1;
  show(bars, prev_bars, "sorted", 1000);
}

void selection_sort(barlist *bars, barlist *prev_bars)
{
  for (int i = 0; i < width; i++)
  {
    bars->selected = i;
    show(bars, prev_bars, "Sorting - Selection sort", step);
    int min = i;
    for (int j = i; j < width; j++)
    {
      if (bars->bars[j] < bars->bars[min])
      {
        bars->selected = j;
        show(bars, prev_bars, "Sorting - Selection sort", step);
        min = j;
      }
    }
    inverse(bars, i, min);
  }
  highlight_all(bars, prev_bars);
}

void bubble_sort(barlist *bars, barlist *prev_bars)
{
  int j = 0;
  while (true)
  {
    bool swapped = false;
    for (int i = 0; i < width - 1 - j; i++)
    {
      if (bars->bars[i] > bars->bars[i + 1])
      {
        bars->selected = i;
        show(bars, prev_bars, "Sorting - Bubble sort", step);
        swapped = true;
        inverse(bars, i, i + 1);
      }
    }
    j++;
    if (!swapped)
      break;
  }
  highlight_all(bars, prev_bars);
}

void merge_sorted_arrays(barlist *bars, barlist *prev_bars, int left, int middle, int right)
{
  int left_len = middle - left + 1;
  int right_len = right - middle;

  bar left_temp[left_len];
  bar right_temp[right_len];

  for (int i = 0; i < left_len; i++)
    left_temp[i] = bars->bars[left + i];
  for (int i = 0; i < right_len; i++)
    right_temp[i] = bars->bars[middle + 1 + i];

  for (int left_count = 0, right_count = 0, total_count = left; total_count <= right; total_count++)
  {
    if ((left_count < left_len) && (right_count >= right_len || left_temp[left_count] <= right_temp[right_count]))
    {
      bars->bars[total_count] = left_temp[left_count];
      left_count++;
    }
    else
    {
      bars->bars[total_count] = right_temp[right_count];
      right_count++;
    }
    bars->selected = total_count;
    show(bars, prev_bars, "Sorting - Merge sorting", step);
  }
}

void merge_sort_recursion(barlist *bars, barlist *prev_bars, int left, int right)
{

  if (left >= right)
    return;

  int middle = left + (right - left) / 2;

  merge_sort_recursion(bars, prev_bars, left, middle);
  merge_sort_recursion(bars, prev_bars, middle + 1, right);

  merge_sorted_arrays(bars, prev_bars, left, middle, right);
}

void merge_sort(barlist *bars, barlist *prev_bars)
{
  merge_sort_recursion(bars, prev_bars, 0, bars->len - 1);
  highlight_all(bars, prev_bars);
}

// Prints the version
void version(void)
{
  printf("%sSorting algorithms visualizer => %s%s%s\n", PURPLE, YELLOW, VERSION, RESET);
}
void help(void)
{
  printf("Sorting algorithms visualizer\n");
  printf("%s━━━━━━━━━━━━━━━━━%s\n", PURPLE, RESET);
  printf("Author: %sSkwalExe%s\n", PURPLE, RESET);
  printf("Github: %shttps://github.com/SkwalExe/%s\n", PURPLE, RESET);
  printf("%s━━━━━━━━━━━━━━━━━%s\n", PURPLE, RESET);
  printf("Terminal utility to visualize sorting algorithms\n");
  printf("%s━━━━━━━━━━━━━━━━━%s\n", PURPLE, RESET);
  printf("Options: \n");
  printf("\t%s-h, --help: %sPrints this message and exits%s\n", PURPLE, YELLOW, RESET);
  printf("\t%s-v, --version: %sPrints the program version and exits%s\n", PURPLE, YELLOW, RESET);
  printf("\t%s-d, --delay: %sSets the delay in ms between each step [D: 15]%s\n", PURPLE, YELLOW, RESET);
  printf("\t%s-s, --size: %sIf the automatic size doesn't fit your terminal, specify it manually with this parameter (ex --size 100 50 -> width of 100 and height of 50)%s\n", PURPLE, YELLOW, RESET);
  printf("\t%s-l, --list: %sPrints the list of algorithms with their ID%s\n", PURPLE, YELLOW, RESET);
  printf("\t%s-i, --id: %sSpecify the ID of the algorithm you want to visualize (-l to print the list, default: 0 for ALL)%s\n", PURPLE, YELLOW, RESET);
  printf("\t%s-p, --precision: %sSpecify the precision of the bars (value goes from one to [p])%s\n", PURPLE, YELLOW, RESET);
}

void list(void)
{
  printf("%sSupported algorithms%s\n", PURPLE, RESET);
  printf("\t%s1 => %sSelection sort%s\n", PURPLE, YELLOW, RESET);
  printf("\t%s2 => %sMerge sort%s\n", PURPLE, YELLOW, RESET);
  printf("\t%s3 => %sBubble sort%s\n", PURPLE, YELLOW, RESET);
}

int main(int argc, char *argv[])
{

  srand((int)clock() + (int)time(NULL));

  // Parse args --------------
  while (--argc > 0)
  {
    char *arg = *++argv;

    if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
    {
      help();
      return 0;
    }
    else if (strcmp(arg, "--version") == 0 || strcmp(arg, "-v") == 0)
    {
      version();
      return 0;
    }
    else if (strcmp(arg, "--list") == 0 || strcmp(arg, "-l") == 0)
    {
      list();
      return 0;
    }
    else if (strcmp(arg, "--delay") == 0 || strcmp(arg, "-d") == 0)
    {
      if (argc <= 1)
      {
        printf("%s[ x ] : An argument is required after -d%s", RED, RESET);
        return 1;
      }

      step = atoi(*++argv);
      argc--;

      if (step <= 0)
      {
        printf("%s[ x ] : The delay must be a number higher than 0%s", RED, RESET);
        return 1;
      }
    }
    else if (strcmp(arg, "--precision") == 0 || strcmp(arg, "-p") == 0)
    {
      if (argc <= 1)
      {
        printf("%s[ x ] : An argument is required after -p%s", RED, RESET);
        return 1;
      }

      precision = atoi(*++argv);
      argc--;

      if (precision <= 2)
      {
        printf("%s[ x ] : The delay must be a number higher than 2%s", RED, RESET);
        return 1;
      }
    }
    else if (strcmp(arg, "--id") == 0 || strcmp(arg, "-i") == 0)
    {
      if (argc <= 1)
      {
        printf("%s[ x ] : An argument is required after -i%s", RED, RESET);
        return 1;
      }

      algorithm = atoi(*++argv);
      argc--;

      if (algorithm < 0 || algorithm > 3)
      {
        printf("%s[ x ] : Invalid algorithm ID%s", RED, RESET);
        return 1;
      }
    }
    else if (strcmp(arg, "--size") == 0 || strcmp(arg, "-s") == 0)
    {
      if (argc <= 2)
      {
        printf("%s[ x ] : Two arguments are required after -s%s", RED, RESET);
        return 1;
      }

      width = atoi(*++argv);
      argc--;
      height = atoi(*++argv);
      argc--;

      if (height <= 1 || width <= 1)
      {
        printf("%s[ x ] : The height and the width must be higher than 1 (and not too high!!)%s", RED, RESET);
        return 1;
      }
    }

    else
    {
      printf("Unknown argument: %s\n", arg);
      return 1;
    }
  }
  // ----------- End of parsing args -------------
  // if the size has not been set manually by the user, determine it automatically
  if (height == 0 && width == 0)
  {
    get_terminal_size(&width, &height);
    height -= TOP_SPACE;
    width /= 2;
  }
  
  clear_terminal();
  hide_cursor();

  set_signal_action();
  setvbuf(stdout, NULL, _IOFBF, 32768);


  barlist bars;
  bars.len = width;
  bars.bars = malloc(sizeof(bar) * width);
  bars.selected = -1;

  barlist prev_bars;
  prev_bars.len = width;
  prev_bars.bars = malloc(sizeof(bar) * width);
  prev_bars.selected = -1;


  for (int i = 0; i < width; i++)
  {
    bars.bars[i] = (1 + i) * precision / width;
    prev_bars.bars[i] = 0;
  }
  

  show(&bars, &prev_bars, "Preparing", 1000);

  highlight_all(&bars, &prev_bars);

  shuffle(&bars, &prev_bars);
  switch (algorithm)
  {
  case 1:
    selection_sort(&bars, &prev_bars);
    break;
  case 2:
    merge_sort(&bars, &prev_bars);
    break;
  case 3:
    bubble_sort(&bars, &prev_bars);
    break;
  default:
    selection_sort(&bars, &prev_bars);
    shuffle(&bars, &prev_bars);
    merge_sort(&bars, &prev_bars);
    shuffle(&bars, &prev_bars);
    bubble_sort(&bars, &prev_bars);
    shuffle(&bars, &prev_bars);
    break;
  }

  free(bars.bars);
  free(prev_bars.bars);
}