#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"

#define RED "\033[91m"
#define YELLOW "\033[93m"
#define PURPLE "\033[95m"
#define WHITE "\033[97m"
#define RESET "\033[0m"

#define VERSION "0.1.0"

typedef int bar;

int width = 0, height = 0;

// 0 => all
// 1 => selection sort
// 2 => merge sort
// 3 => bubble sort
int algorithm = 0;

int step = 15;

// Inverse bar at index a with bar at index b
void inverse(bar bars[width], int a, int b)
{
  bar temp = bars[a];
  bars[a] = bars[b];
  bars[b] = temp;
}

// print bars for ms milliseconds
// focused_index = -1 to ingore
void show(bar bars[width], int focused_index, char *text, int ms)
{
  clear_terminal();
  printf("%s\n", text);
  for (int y = height; y >= 0; y--)
  {
    for (int x = 0; x < width; x++)
    {
      // height relative to term height
      float h = height * bars[x] / 100.0;
      if (h > y)
      {
        printf("\x1b[4%cm  \x1b[0m", focused_index == x ? '5' : '7');
      }
      else
      {
        printf("  ");
      }
    }
    printf("\n");
  }
  sleep_ms(ms);
}
void shuffle(bar bars[width])
{
  for (int x = 0; x < width; x++)
  {
    int swap_with = random_int(0, width - 1);
    inverse(bars, x, swap_with);
    show(bars, swap_with, "shuffling", step);
  }

  show(bars, -1, "Preparing", 1000);
}

void highlight_all(bar bars[width])
{
  for (int i = 0; i < width; i++)
  {
    show(bars, i, "sorted", step);
  }
  show(bars, -1, "sorted", 1000);
}

void selection_sort(bar bars[width])
{
  for (int i = 0; i < width; i++)
  {
    show(bars, i, "Sorting - Selection sort", step);
    int min = i;
    for (int j = i; j < width; j++)
    {
      if (bars[j] < bars[min])
      {
        show(bars, j, "Sorting - Selection sort", step);
        min = j;
      }
    }
    inverse(bars, i, min);
  }
  highlight_all(bars);
}

void bubble_sort(bar bars[width])
{
  int j = 0;
  while (true)
  {
    bool swapped = false;
    for (int i = 0; i < width - 1 - j; i++)
    {
      if (bars[i] > bars[i + 1])
      {
        show(bars, i, "Sorting - Bubble sort", step);
        swapped = true;
        inverse(bars, i, i + 1);
      }
    }
    j++;
    if (!swapped)
      break;
  }
  highlight_all(bars);
}

void merge_sorted_arrays(bar bars[], int left, int middle, int right)
{
  int left_len = middle - left + 1;
  int right_len = right - middle;

  bar left_temp[left_len];
  bar right_temp[right_len];

  for (int i = 0; i < left_len; i++)
    left_temp[i] = bars[left + i];
  for (int i = 0; i < right_len; i++)
    right_temp[i] = bars[middle + 1 + i];

  for (int left_count = 0, right_count = 0, total_count = left; total_count <= right; total_count++)
  {
    if ((left_count < left_len) && (right_count >= right_len || left_temp[left_count] <= right_temp[right_count]))
    {
      bars[total_count] = left_temp[left_count];
      left_count++;
    }
    else
    {
      bars[total_count] = right_temp[right_count];
      right_count++;
    }
    show(bars, total_count, "Sorting - Merge sorting", step);
  }
}

void merge_sort_recursion(bar bars[], int left, int right)
{

  if (left >= right)
    return;

  int middle = left + (right - left) / 2;

  merge_sort_recursion(bars, left, middle);
  merge_sort_recursion(bars, middle + 1, right);

  merge_sorted_arrays(bars, left, middle, right);
}

void merge_sort(bar bars[], int len)
{
  merge_sort_recursion(bars, 0, len - 1);
  highlight_all(bars);
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
    height -= 4;
    width /= 2;
  }

  bar bars[width];

  for (int i = 0; i < width; i++)
  {
    bars[i] = (1 + i) * 100 / width;
  }

  highlight_all(bars);
  shuffle(bars);

  switch (algorithm)
  {
  case 1:
    selection_sort(bars);
    break;
  case 2:
    merge_sort(bars, width);
    break;
  case 3:
    bubble_sort(bars);
    break;
  default:
    selection_sort(bars);
    shuffle(bars);
    merge_sort(bars, width);
    shuffle(bars);
    bubble_sort(bars);
    shuffle(bars);
    break;
  }
}