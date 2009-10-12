// cartograms.h
// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)

#ifndef INCLUDE_GUARD_CARTOGRAMS_H
#define INCLUDE_GUARD_CARTOGRAMS_H

#include <vector>
#include <string>
#include "../shared/vector2.h"

namespace hashimoto_ut {

  class Thread;

  namespace cartograms {

    // @density is a column-major array of the density on representative coordinates.
    std::vector<Vector2<double> >
      generate(Thread* parent, std::wstring* status,
               std::vector<double> const& density, int xsize, int ysize, double blur);

  }

}

#endif // INCLUDE_GUARD_CARTOGRAMS_H
