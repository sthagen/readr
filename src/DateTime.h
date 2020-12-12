#ifndef READR_DATE_TIME_H_
#define READR_DATE_TIME_H_

#include "cpp11/R.hpp"

#include "localtime.h"
#include <ctime>
#include <stdlib.h>
#include <string>

// Much of this code is adapted from R's src/main/datetime.c.
// Author: The R Core Team.
// License: GPL >= 2

static const int month_length[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static const int month_start[12] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

// Leap days occur in a 400 year cycle: this records the cumulative number
// of leap days in per cycle. Generated with:
// is_leap <- function(y) (y %% 4) == 0 & ((y %% 100) != 0 | (y %% 400) == 0)
// cumsum(is_leap(0:399))
static const int leap_days[400] = {
    0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,  4,  4,  4,  4,  5,  5,
    5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9,  10,
    10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14,
    15, 15, 15, 15, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19,
    19, 20, 20, 20, 20, 21, 21, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24,
    24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 27, 28,
    28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 30, 31, 31, 31, 31, 32, 32, 32, 32,
    33, 33, 33, 33, 34, 34, 34, 34, 35, 35, 35, 35, 36, 36, 36, 36, 37, 37, 37,
    37, 38, 38, 38, 38, 39, 39, 39, 39, 40, 40, 40, 40, 41, 41, 41, 41, 42, 42,
    42, 42, 43, 43, 43, 43, 44, 44, 44, 44, 45, 45, 45, 45, 46, 46, 46, 46, 47,
    47, 47, 47, 48, 48, 48, 48, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50,
    51, 51, 51, 51, 52, 52, 52, 52, 53, 53, 53, 53, 54, 54, 54, 54, 55, 55, 55,
    55, 56, 56, 56, 56, 57, 57, 57, 57, 58, 58, 58, 58, 59, 59, 59, 59, 60, 60,
    60, 60, 61, 61, 61, 61, 62, 62, 62, 62, 63, 63, 63, 63, 64, 64, 64, 64, 65,
    65, 65, 65, 66, 66, 66, 66, 67, 67, 67, 67, 68, 68, 68, 68, 69, 69, 69, 69,
    70, 70, 70, 70, 71, 71, 71, 71, 72, 72, 72, 72, 73, 73, 73, 73, 73, 73, 73,
    73, 74, 74, 74, 74, 75, 75, 75, 75, 76, 76, 76, 76, 77, 77, 77, 77, 78, 78,
    78, 78, 79, 79, 79, 79, 80, 80, 80, 80, 81, 81, 81, 81, 82, 82, 82, 82, 83,
    83, 83, 83, 84, 84, 84, 84, 85, 85, 85, 85, 86, 86, 86, 86, 87, 87, 87, 87,
    88, 88, 88, 88, 89, 89, 89, 89, 90, 90, 90, 90, 91, 91, 91, 91, 92, 92, 92,
    92, 93, 93, 93, 93, 94, 94, 94, 94, 95, 95, 95, 95, 96, 96, 96, 96, 97, 97,
    97};

static const int cycle_days = 400 * 365 + 97;

inline int is_leap(unsigned y) {
  return (y % 4) == 0 && ((y % 100) != 0 || (y % 400) == 0);
}

class DateTime {
  int year_, mon_, day_, hour_, min_, sec_, offset_;
  double psec_;
  std::string tz_;

public:
  DateTime(
      int year,
      int mon,
      int day,
      int hour = 0,
      int min = 0,
      int sec = 0,
      double psec = 0,
      const std::string& tz = "")
      : year_(year),
        mon_(mon),
        day_(day),
        hour_(hour),
        min_(min),
        sec_(sec),
        offset_(0),
        psec_(psec),
        tz_(tz) {}

  // Used to add time zone offsets which can only be easily applied once
  // we've converted into seconds since epoch.
  void setOffset(int offset) { offset_ = offset; }

  // Is this a valid date time?
  bool validDateTime() const { return validDate() && validTime(); }

  bool validDate() const {
    if (year_ < 0)
      return false;
    if (mon_ < 0 || mon_ > 11)
      return false;
    if (day_ < 0 || day_ >= days_in_month())
      return false;

    return true;
  }

  bool validTime() const {
    if (sec_ < 0 || sec_ > 60)
      return false;
    if (min_ < 0 || min_ > 59)
      return false;
    if (hour_ < 0 || hour_ > 23)
      return false;

    return true;
  }

  bool validDuration() const {
    if (sec_ < -59 || sec_ > 59)
      return false;
    if (min_ < -59 || min_ > 59)
      return false;

    return true;
  }

  double datetime() const { return (tz_ == "UTC") ? utctime() : localtime(); }

  int date() const { return utcdate(); }

  double time() const {
    return psec_ + sec_ + (min_ * 60.0) + (hour_ * 3600.0);
  }

private:
  // Number of number of seconds since 1970-01-01T00:00:00Z.
  // Compared to usual implementations this returns a double, and supports
  // a wider range of dates. Invalid dates have undefined behaviour.
  double utctime() const { return utcdate() * 86400.0 + time() + offset_; }

  // Find number of days since 1970-01-01.
  // Invalid dates have undefined behaviour.
  int utcdate() const {
    if (!validDate())
      return NA_REAL;

    // Number of days since start of year
    int day = month_start[mon_] + day_;
    if (mon_ > 1 && is_leap(year_))
      day++;

    // Number of days since 0000-01-01
    // Leap years come in 400 year cycles so determine which cycle we're
    // in, and what position we're in within that cycle.
    int ly_cycle = year_ / 400;
    int ly_offset = year_ - (ly_cycle * 400);
    if (ly_offset < 0) {
      ly_offset += 400;
      ly_cycle--;
    }
    day += ly_cycle * cycle_days + ly_offset * 365 + leap_days[ly_offset];

    // Convert to number of days since 1970-01-01
    day -= 719528;

    return day;
  }

  double localtime() const {
    if (!validDateTime())
      return NA_REAL;

    struct Rtm tm;
    tm.tm_year = year_ - 1900;
    tm.tm_mon = mon_;
    tm.tm_mday = day_ + 1;
    tm.tm_hour = hour_;
    tm.tm_min = min_;
    tm.tm_sec = sec_;
    // The Daylight Saving Time flag (tm_isdst) is greater than zero if Daylight
    // Saving Time is in effect, zero if Daylight Saving Time is not in effect,
    // and less than zero if the information is not available.
    tm.tm_isdst = -1;

    time_t time = my_mktime(&tm, tz_.c_str());
    return time + psec_ + offset_;
  }

  inline int days_in_month() const {
    return month_length[mon_] + (mon_ == 1 && is_leap(year_));
  }
  inline int days_in_year() const { return 365 + is_leap(year_); }
};

#endif
