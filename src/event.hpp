/*
 * vi:ts=4:shiftwidth=4:expandtab
 * vim600:fdm=marker
 *
 * event.hpp  -  define an event (sample) in the event space of the MaxentModel
 *
 * Copyright (C) 2003 by Zhang Le <ejoy@users.sourceforge.net>
 * Begin       : 01-Jan-2003
 * Last Change : 24-Sep-2003.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef EVENT_H
#define EVENT_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cassert>
#include <vector>
#include <utility>
#include <ext/algorithm>
#include <boost/utility.hpp>
// #include <SmallObj.h>

#include "itemmap.hpp"

namespace maxent {

struct Event /*: public Loki::SmallObject<>*/ {
    public:
    typedef size_t    pred_id_type;
    typedef size_t outcome_id_type;
    // to save memory, feature value is defined as float
    typedef pair<pred_id_type, float> context_type;

    Event():m_context(0), m_context_size(0),m_outcome(0), m_count(0){}
    Event( context_type* context,
            size_t context_size,
            outcome_id_type oid,
            size_t count):
        m_context(context), m_context_size(context_size),
        m_outcome(oid), m_count(count) {}

    context_type*    m_context;
    size_t           m_context_size;
    outcome_id_type  m_outcome;
    size_t           m_count;

    size_t context_size() const { return m_context_size; }

    //first compare context (including fvalues) then outcome
    bool operator<(const Event& rhs) const {
        int ret = __gnu_cxx::lexicographical_compare_3way(m_context,
                m_context + m_context_size,
                rhs.m_context, rhs.m_context + rhs.m_context_size);
        if (ret == 0)
            return m_outcome < rhs.m_outcome;
        else
            return ret < 0;
    }

    bool operator>(const Event& rhs) const {
        return rhs < *this;
    }

    // two events are equal if they have same context and outcome
    bool operator==(const Event& rhs) const {
        return m_outcome == rhs.m_outcome && is_same_context(rhs);
    }

    bool is_same_context(const Event& rhs) const {
            return (__gnu_cxx::lexicographical_compare_3way(m_context,
                m_context + m_context_size,
                rhs.m_context, rhs.m_context + rhs.m_context_size) == 0); 
    }
};

// this class is responsible to free memory allocated for context on destory
// it's noncopyable, so can only has one instance and be used in shared_ptr<>
// when deleting elements from this vector, the caller is responsible to free
// those elements
#include <iostream>
class EventVector : public std::vector<Event>, public boost::noncopyable {
    public:
        virtual ~EventVector() {
            for (size_t i = 0; i < size(); ++i) {
                 delete[] (*this)[i].m_context;
//     Loki::SmallObject<>::operator delete(
//             (*this)[i].m_context, (*this)[i].context_size()*sizeof(size_t));
            }
        }
};

typedef size_t outcome_id_type;
typedef std::string feature_type;
typedef std::string outcome_type;
typedef ItemMap<feature_type> PredMapType;
typedef ItemMap<outcome_type> OutcomeMapType;
typedef std::vector<std::vector<pair<size_t, size_t> > > ParamsType;

} // namespace maxent
#endif /* ifndef EVENT_H */

