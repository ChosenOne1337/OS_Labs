#ifndef LONGRANGE_H
#define LONGRANGE_H

typedef struct Range {
    long lowerBound;
    long extent;
} Range;

Range range_make(long lowerBound, long extent);

void range_resize(Range *range, long newExtent);
void range_set_lower_bound(Range *range, long newLowerBound);

long range_get_lower_bound(Range *range);
long range_get_upper_bound(Range *range);
long range_get_extent(Range *range);

#endif // LONGRANGE_H
