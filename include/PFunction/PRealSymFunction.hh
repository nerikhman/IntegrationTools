
#ifndef PRealSymFunction_HH
#define PRealSymFunction_HH

#include "PFunction.hh"
#include "PSeriesFunction.hh"
#include <ginac/ginac.h>    // compile with: -lcln -lginac

namespace PRISMS
{

    /// Real valued symbolic functions

    template<class VarContainer, class IndexContainer>
    class PRealSymFunction : public PBaseFunction<double, double, VarContainer, IndexContainer>
    {
        GiNaC::ex _e;
        std::vector< GiNaC::symbol> _sym;

        double _val;
        std::vector<double> _grad;
        std::vector< std::vector<double> > _hess;

    public:

        // ----------------------------------------------------------
        //   Non-Inherited:

        void set(const std::string &name, const std::vector<GiNaC::symbol> &sym, const GiNaC::ex &e)
        {
            _sym.clear();
            _sym.resize(_sym.size());

            _var_name.clear();
            _var_name.resize(_sym.size());

            for(int i = 0; i < sym.size(); i++)
            {
                _sym.push_back(sym[i]);
                _sym[i] = sym[i];
                _var_name.push_back(_sym[i].get_name());
            }

            _name = name;

            _e = e;
        }


        // ----------------------------------------------------------
        //   Inherited:

        using PBaseFunction<double, double, VarContainer, IndexContainer>::_name;
        using PBaseFunction<double, double, VarContainer, IndexContainer>::_var_name;

        virtual PRealSymFunction<VarContainer, IndexContainer> *clone() const
        {
            return new PRealSymFunction<VarContainer, IndexContainer>(*this);
        };

        // ----------------------------------------------------------
        // Use these functions if you want to evaluate a single value
        double operator()(const VarContainer &var) const;
        double grad(const VarContainer &var, int di) const;
        double hess(const VarContainer &var, int di, int dj) const;

    };
    
    

    template<class VarContainer, class IndexContainer>
    double PRealSymFunction<VarContainer, IndexContainer>::operator()(const VarContainer &var) const
    {
        GiNaC::exmap m;
        for(int i = 0; i < var.size(); i++)
            m[_sym[i]] = var[i];
        
        return GiNaC::ex_to<GiNaC::numeric>(GiNaC::evalf(_e.subs(m))).to_double();
    }


    template<class VarContainer, class IndexContainer>
    double PRealSymFunction<VarContainer, IndexContainer>::grad(const VarContainer &var, int di) const
    {
        GiNaC::ex de = _e.diff(_sym[di]);

        GiNaC::exmap m;
        for(int i = 0; i < var.size(); i++)
            m[_sym[i]] = var[i];
        
        return GiNaC::ex_to<GiNaC::numeric>(GiNaC::evalf(de.subs(m))).to_double();
    };

    template<class VarContainer, class IndexContainer>
    double PRealSymFunction<VarContainer, IndexContainer>::hess(const VarContainer &var, int di, int dj) const
    {
        GiNaC::ex de = _e.diff(_sym[di]).diff(_sym[dj]);

        GiNaC::exmap m;
        for(int i = 0; i < var.size(); i++)
            m[_sym[i]] = var[i];
        
        return GiNaC::ex_to<GiNaC::numeric>(GiNaC::evalf(de.subs(m))).to_double();
    };

    // ----------------------------------------------------------
    
    
    
    class PRealSymBasisFunction : public PBasisFunction<double, double>
    {
        public:
        GiNaC::ex _e;
        GiNaC::symbol _var;
        using PBasisFunction<double, double>::_val;
        
        // Reminder, inherited from PSymBasisFunction<double, double>:
        //double operator()( const double &var){ _val = eval(var); return _val;};
        //double operator()(){ return _val;};
        
        PRealSymBasisFunction()
        {
        };
        
        PRealSymBasisFunction( GiNaC::symbol var, GiNaC::ex e)
        {
            _var = var;
            _e = e;
        };
        
        private:
        virtual double eval( const double &var)
        { 
            return GiNaC::ex_to<GiNaC::numeric>(GiNaC::evalf(_e.subs(_var == var))).to_double();
        };
        
        
    };
    
    /// Generate PRealSymBasisFunction objects, assuming they can be written in the form:
    ///
    ///   phi_i(x) = f(i, x);
    ///
    ///   For instance, simple polynomials:  phi_i(x) = x^i;
    ///
    class PRealSymBasisFunctionFactory : public PBasisFunctionFactory<double, double>
    {
        public:
        GiNaC::ex _e;           // expression defining the basis functions = f( _var, _i)
        GiNaC::symbol _var;     // variable
        GiNaC::symbol _i;       // index of basis function
        
        PRealSymBasisFunctionFactory( const GiNaC::symbol &index, const GiNaC::symbol &var, const GiNaC::ex &e)
        {
            _e = e;
            _var = var;
            _i = index;
        };
        
        virtual PRealSymBasisFunction* new_basis_function( int term) const
        {
            return new PRealSymBasisFunction( _var, _e.subs(_i == term));
        };
        
        virtual PRealSymBasisFunction* new_basis_grad_function( int term)const
        {
            return new PRealSymBasisFunction( _var, _e.subs(_i == term).diff(_var) );
        };
        
        virtual PRealSymBasisFunction* new_basis_hess_function( int term)const
        {
            return new PRealSymBasisFunction( _var, _e.subs(_i == term).diff(_var, 2));
        };
        
    };
    
    /// Generate PRealSymBasisFunction objects, assuming they can be written recursively in the form:
    ///
    ///   phi_i(x) = f(i, x, phi_i-1, phi_i-2, ... phi_i-d);  where d is depth of recursion
    ///
    ///   For instance, Chebyshev polynomials of the first kind:  
    ///      phi_0(x) = 1;
    ///      phi_1(x) = x;
    ///      phi_n(x) = 2*x*phi_n-1(x) - phi_n-2(x); with d = 2
    ///
    class PRealSymRecursBasisFunctionFactory : public PBasisFunctionFactory<double, double>
    {
        public:
        std::vector< GiNaC::ex> _phi;  // generated basis functions
        std::vector< GiNaC::ex> _phi_sym; // symbols used for phi_n-1, phi_n-2, etc. in _e_gen
        GiNaC::ex _e_gen;       // generating expression defining the basis functions
        GiNaC::symbol _var;     // variable
        
        
        /// 'phi_init' contains the necessary phi_0(var), phi_1(var), ... phi_depth-1(var)
        /// 'e_gen' is the generating expression, for Chebyshev: '2*x*phi1 - phi0'
        /// 'phi_sym' is an array containing the symbols used for previous basis functions,
        /// 'phi_sym' and 'phi_init' are arrays of length 'depth'
        ///
        /// For example, the first 100 Chebyshev polynomials could be generated using:
        ///   double depth = 2; 
        ///   GiNaC::symbol x("x"); 
        ///   GiNaC::ex phi_sym[2] = {phi0, phi1};
        ///   GiNaC::ex phi_init[2] = {1, x};
        ///   GiNaC::ex e_gen = 2*x*phi1 - phi0;
        ///   PRealSymBasisFunctionFactory chebyshev_factory( depth, x, phi_sym, phi_init, e_gen);
        ///   
        ///   int N = 100;
        ///   std::vector<PRealSymBasisFunction*> chebyshev;
        ///   chebyshev.resize(N, NULL);
        ///   for( int i = 0; i < N; i++)
        ///     chebyshev[i] = chebyshev_factory.new_basis_function(i);
        ///
        ///   // ... use them ...
        ///   
        ///   // remember to delete anything created by a factory
        ///   for( int i = 0; i < chebyshev.size; i++)
        ///     delete chebyshev[i];
        ///
        PRealSymRecursBasisFunctionFactory( int depth, const GiNaC::symbol &var, GiNaC::symbol *phi_sym, GiNaC::ex *phi_init, const GiNaC::ex &e_gen)
        {
            _e_gen = e_gen;
            _var = var;
            
            _phi_sym.resize(depth);
            _phi.resize(depth);
            
            for( int i=0; i<depth; i++)
            {
                _phi_sym[i] = phi_sym[i];
                _phi[i] = phi_init[i];
            }
        };
        
        virtual PRealSymBasisFunction* new_basis_function( int term)
        {
            generate_up_to(term);
            return new PRealSymBasisFunction( _var, _phi[term]);
        };
        
        virtual PRealSymBasisFunction* new_basis_grad_function( int term)
        {
            generate_up_to(term);
            return new PRealSymBasisFunction( _var, _phi[term].diff(_var) );
        };
        
        virtual PRealSymBasisFunction* new_basis_hess_function( int term)
        {
            generate_up_to(term);
            return new PRealSymBasisFunction( _var, _phi[term].diff(_var, 2));
        };
        
        private:
        
        void generate_up_to( int term)
        {
            while( _phi.size() <= term)
            {
                GiNaC::exmap m;
                for(int i = 0; i < _phi_sym.size(); i++)
                {
                    m[_phi_sym[i]] = _phi[_phi.size()-_phi_sym.size()+i];
                }
                _phi.push_back(_e_gen.subs(m, GiNaC::subs_options::algebraic).expand());
            }
        };
        
    };
    

}


#endif