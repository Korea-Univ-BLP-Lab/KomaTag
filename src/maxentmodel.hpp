/**
 * vi:ts=4:shiftwidth=4:expandtab
 * vim600:fdm=marker
 *
 * maxentmodel.hpp  -  A Conditional Maximun Entropy Model
 *
 * Copyright (C) 2003 by Zhang Le <ejoy@users.sourceforge.net>
 * Begin       : 01-Jan-2003
 * Last Change : 06-Oct-2003.
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

#ifndef MAXENTMODEL_H
#define MAXENTMODEL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>
#include <utility>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <ostream>
#include <iostream>

#include "itemmap.hpp"
#include "event.hpp"

#if defined(PYTHON_MODULE)
    #include <boost/python/class.hpp>
    #include <boost/python/object.hpp>
    #include <boost/python/list.hpp>
    #include <boost/python/tuple.hpp>
    #include <boost/python/extract.hpp>
    #include <boost/python/type_id.hpp>
namespace py = boost::python;
using py::extract;
#endif

/**
 * All classes and functions are placed in the namespace maxent.
 */
namespace maxent {
using namespace std;
using boost::shared_ptr;
using boost::shared_array;

extern int verbose;  // set this to 0 if you do not want verbose output
struct maxent_pickle_suite;
/**
 * This class implement a conditional Maximun Entropy Model.
 *
 * A conditional Maximun Entropy Model has the form:
 * \f$p(y|x)=\frac{1}{Z(x)} \exp \left[\sum_{i=1}^k\lambda_if_i(x,y) \right]\f$
 * Where x is a context and y is the outcome tag and p(y|x) is the conditional
 * probability.
 *
 * Normally the context x is composed of a set of contextual predicates.
 */
class MaxentModel /*: TODO: we need copyable? boost::noncopyable*/  {
    friend struct maxent_pickle_suite;

    // private:
    // virtual ~MaxentModel();

    public:
    typedef std::string feature_type;
    typedef std::string outcome_type;
    typedef std::vector<pair<feature_type, float> > context_type;


    MaxentModel();

    void load(const string& model, const string& param = "");

    void save(const string& model, bool binary = false) const;

    double eval(const context_type& context, const outcome_type& outcome) const;

    void eval_all(const context_type& context,
            std::vector<pair<outcome_type, double> >& outcomes,
            bool sort_result = true) const;

    outcome_type predict(const context_type& context) const;

    void begin_add_event();

    void add_event(const context_type& context,
            const outcome_type& outcome,
            size_t count = 1);

    // wrapper functions for binary feature cases, provided for conviences
    void add_event(const vector<string>& context,
            const outcome_type& outcome,
            size_t count = 1);

    double eval(const vector<string>& context, const outcome_type& outcome) const;

    void eval_all(const vector<string>& context,
            std::vector<pair<outcome_type, double> >& outcomes,
            bool sort_result = true) const;

    outcome_type predict(const vector<string>& context) const;

    /**
     * Add a set of events indicated by range [begin, end).
     * the value type of Iterator must be pair<context_type, outcome_type>
     */
    template <typename Iterator>
        void add_events(Iterator begin, Iterator end) {
            for (Iterator it = begin; it != end; ++it)
                this->add_event(it->first, it->second);
        }

    void add_heldout_event(const context_type& context,
            const outcome_type& outcome,
            size_t count = 1);

    void end_add_event(size_t cutoff = 1);

    void train(size_t iter = 15, const std::string& method = "lbfgs",
            double sigma = 0.0, // non-zero enables Gaussian prior smoothing
            double tol = 1E-05);

    void dump_events(const string& model, bool binary = false) const;

    string __str__() const;

    // py binding {{{
#if defined(PYTHON_MODULE)
    // convert python object: ['str', ] or [('str', float), ] into c++
    // context_type
    void convert_py_context(py::object py_context, context_type& context) const {
        using py::extract;
        if (!context.empty())
            context.clear();

        size_t len = extract<size_t>(py_context.attr("__len__")());
        string pred;
        if (len > 0) {
            string type = extract<std::string>(py_context[0].attr("__class__").attr("__name__"));
            if (type == "tuple") { // pair<string, double> 
                for (size_t i = 0;i < len; ++i) {
                    pred =  extract<std::string>(py_context[i][0]);
                    double fval = extract<double>(py_context[i][1]);
                    context.push_back(make_pair(pred, fval));
                }
            } else {
                for (size_t i = 0;i < len; ++i) {
                    pred = extract<std::string>(py_context[i]);
                    context.push_back(make_pair(pred, 1.0));
                }
            }
        }
    }

    py::list py_eval(py::object py_context) const {
        context_type context;
        convert_py_context(py_context, context);

        py::list result;
        static std::vector<pair<outcome_type, double> > outcomes;

        eval_all(context, outcomes);
        vector<pair<outcome_type, double> >::iterator it;
        for (it = outcomes.begin(); it != outcomes.end(); ++it){
            result.append(py::make_tuple(it->first, it->second));
        }
        return result;
    }

    void py_add_event(py::object py_context, 
            const outcome_type& outcome,
            size_t count = 1) {
        context_type context;
        convert_py_context(py_context, context);
        add_event(context, outcome, count);
    }

    outcome_type py_predict(py::object py_context) const {
        context_type context;
        convert_py_context(py_context, context);
        return predict(context);
    }
#endif
    // end py binding }}}

    private:
    void merge_events(shared_ptr<EventVector > events,
            size_t cutoff) const;

    double build_params(shared_ptr<ParamsType>& params, size_t& n_theta) const;
    double build_params2(shared_ptr<ParamsType>& params, size_t& n_theta) const;

    struct featid_hasher {
        size_t operator()(const pair<size_t, size_t>& p) const {
            return p.first + p.second;
        }
    };

    struct cutoffed_event {
        cutoffed_event(size_t cutoff):m_cutoff(cutoff) {}
        bool operator()(const Event& ev) const {
            return ev.m_count < m_cutoff;
        }
        size_t m_cutoff;
    };

    struct cmp_outcome {
        bool operator()(const pair<outcome_type, double>& lhs,
                const pair<outcome_type, double>& rhs) const {
            return lhs.second > rhs.second;
        }
    };

    size_t m_n_theta;
    shared_ptr<ItemMap<feature_type> > m_pred_map;
    shared_ptr<ItemMap<outcome_type> > m_outcome_map;
    shared_ptr<ParamsType > m_params;
    shared_ptr<EventVector > m_events;
    shared_ptr<EventVector > m_heldout_events;
    shared_array<double> m_theta; // feature weights
    // shared_array<double> m_sigma; // Gaussian prior

    struct param_hasher {
        size_t operator()(const pair<size_t,size_t>& v) const {
            return size_t(~(v.first<< 1) + v.second);
        }
    };
};

#if defined(PYTHON_MODULE) //{{{
struct maxent_pickle_suite : boost::python::pickle_suite {
    static boost::python::tuple getstate(const MaxentModel& m)
    {
        if (!m.m_params)
            throw runtime_error("can not get state from empty model");
        using namespace boost::python;
        list state;
        size_t i;

        shared_ptr<ItemMap<std::string> > pred_map = m.m_pred_map;
        shared_ptr<ItemMap<std::string> > outcome_map = m.m_outcome_map;
        shared_ptr<ParamsType > params = m.m_params;
        size_t n_theta = m.m_n_theta;
        shared_array<double> theta = m.m_theta;

        // save pred_map
        state.append(pred_map->size());
        for (i = 0;i < pred_map->size(); ++i)
            state.append((*pred_map)[i]);

        // save outcome_map
        state.append(outcome_map->size());
        for (i = 0;i < outcome_map->size(); ++i)
            state.append((*outcome_map)[i]);

        // save params
        state.append(n_theta);
        assert(params->size() == pred_map->size());
        for (i = 0;i < params->size(); ++i) {
            list oids;
            list t;
            const std::vector<pair<size_t, size_t> >& a = (*params)[i];
            for (size_t j = 0; j < a.size(); ++j) {
                oids.append(a[j].first);
                t.append(a[j].second);
            }
            state.append(make_tuple(oids, t));
        }
        // save theta
        for (i = 0;i < n_theta; ++i)
            state.append(theta[i]);
        return boost::python::tuple(state);
    }

    static void setstate(MaxentModel& m, boost::python::tuple state)
    {
        using namespace boost::python;
        assert (!m.m_pred_map);
        assert (!m.m_outcome_map);
        assert (!m.m_params);
        assert (len(state) > 0);

        shared_ptr<ItemMap<std::string> > pred_map(new ItemMap<std::string>);
        shared_ptr<ItemMap<std::string> > outcome_map(new ItemMap<std::string>);
        shared_ptr<ParamsType > params( new ParamsType);
        size_t n_theta;
        shared_array<double> theta;

        size_t count;
        size_t i;
        size_t index = 0;

        // load pred_map
        count = extract<size_t>(state[index++]);
        for (i = 0; i < count; ++i)
            pred_map->add(extract<std::string>(state[index++]));

        // load outcome_map
        count = extract<size_t>(state[index++]);
        for (i = 0; i < count; ++i)
            outcome_map->add(extract<std::string>(state[index++]));

        // load params
        n_theta = extract<size_t>(state[index++]);
        for (i = 0; i < pred_map->size(); ++i) {
            tuple tmp(state[index++]);
            list oids(tmp[0]);
            list t(tmp[1]);
            std::vector<pair<size_t, size_t> > a;

            size_t k = extract<size_t>(oids.attr("__len__")());
            assert (k == len(t));

            for (size_t j = 0; j < k; ++j) {
                size_t oid = extract<size_t>(oids[j]);
                size_t fid = extract<size_t>(t[j]);
                a.push_back(std::make_pair(oid, fid));
            }
            params->push_back(a);
        }
        // extract theta
        theta.reset(new double[n_theta]);
        for (i = 0;i < n_theta; ++i)
            theta[i] = extract<double>(state[index++]);
        m.m_pred_map = pred_map;
        m.m_outcome_map = outcome_map;
        m.m_params = params;
        m.m_n_theta = n_theta;
        m.m_theta = theta;
    }
};
#endif // PYTHON_MODULE }}}

} // namespace maxent
#endif /* ifndef MAXENTMODEL_H */

