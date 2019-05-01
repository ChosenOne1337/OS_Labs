#include "range.h"

Range range_make(long lowerBound, long extent) {
    Range range = {
        .lowerBound = lowerBound,
        .extent = extent
    };
    return range;
}


void range_resize(Range *range, long newExtent) {
    range->extent = newExtent;
}

void range_set_lower_bound(Range *range, long newLowerBound) {
    range->lowerBound = newLowerBound;
}


long range_get_lower_bound(Range *range) {
    return range->lowerBound;
}

long range_get_upper_bound(Range *range) {
    return range->lowerBound + range->extent;
}

long range_get_extent(Range *range) {
    return range->extent;
}
