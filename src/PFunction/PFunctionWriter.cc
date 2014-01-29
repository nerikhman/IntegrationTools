
#ifndef PFunctionWriter_CC
#define PFunctionWriter_CC

#include<cstring>
#include<iostream>
#include<sstream>
#include<vector>
#include<time.h>
#include<cstdlib>
#include <ginac/ginac.h>    // compile with: -lcln -lginac

#include "PFunctionWriter.hh"
#include "version.hh"

namespace PRISMS
{
    // convert int to string
    std::string itos( int i)
    {
        std::stringstream ss;
        ss << i;
        return ss.str();
    }
    
    // return current date and time as: YEAR-MM-DD HH:MM:SS
    std::string now()
    {
        time_t t = time(0);   // get time now
        struct tm * _now = localtime( & t );
        std::ostringstream ss;
        
        std::string smin = "";
        int min = _now->tm_min;
        if( min < 10)
            smin = "0" + itos(min);
        else
            smin = itos(min);
        
        std::string ssec = "";
        int sec = _now->tm_sec;
        if( sec < 10)
            ssec = "0" + itos(sec);
        else
            ssec = itos(sec);
        
                
        ss << (_now->tm_year + 1900) << '-' 
             << (_now->tm_mon + 1) << '-'
             <<  _now->tm_mday
             << " " << _now->tm_hour << ":" << smin << ":" << ssec;
        return ss.str();
        
    }


    PFunctionWriter::PFunctionWriter(const std::string &name)
    {
        _name = name;
        
        // set defaults:
        template_intype_on();
        f_on();
        grad_on();
        hess_on();
        set_basic_indent("    ");
        set_types("double", "double");
    }
    
    void PFunctionWriter::set_basic_indent( std::string basic_indent)
    {
        _basic_indent = basic_indent;
    }
    
    void PFunctionWriter::set_intype(std::string intype)
    {
        _intype = intype;
    }
    
    void PFunctionWriter::set_outtype(std::string outtype)
    {
        _outtype = outtype;
    }
    
    void PFunctionWriter::set_types( std::string intype, std::string outtype)
    {
        _intype = intype;
        _outtype = outtype;
    }
    
    void PFunctionWriter::template_intype_on()
    {
        _template_intype = true;
    }
    
    void PFunctionWriter::template_intype_off()
    {
        _template_intype = false;
    }
    
    void PFunctionWriter::f_on()
    {
        _write_f = true;
    }
    
    void PFunctionWriter::f_off()
    {
        _write_f = false;
    }
    
    void PFunctionWriter::grad_on()
    {
        _write_grad = true;
    }
    
    void PFunctionWriter::grad_off()
    {
        _write_grad = false;
    }
    
    void PFunctionWriter::hess_on()
    {
        _write_hess = true;
    }
    
    void PFunctionWriter::hess_off()
    {
        _write_hess = false;
    }
    
    void PFunctionWriter::set_var( const std::vector< std::string> &var_name, 
                  const std::vector< std::string> &var_description)
    {
        if( var_name.size() != var_description.size())
        {
            std::cout << "Error in PFunctionWriter::set_var" << std::endl;
            std::cout << "  var_name.size() != var_description.size()" << std::endl;
            exit(1);
        }
        _var_name = var_name;
        _var_description = var_description;
    }


    /*
    void PFunctionWriter::sym( std::string f, std::ostream &sout)
    {
        std::cout << "PFunctionWriter::sym( std::string f, std::ostream &sout) is undefined" << std::endl;
        exit(1);
    }
    */
    
    void PFunctionWriter::sym2code( const std::string &f, std::ostream &sout)
    {
        std::cout << "Input f = " << f << std::endl;
        
        // read 'f' into a GiNaC expression 'symf', 
        //    using the variables names already given
        GiNaC::symtab table;
        GiNaC::symtab all_symbol_table;
        std::vector<GiNaC::symbol> symvar(_var_name.size());
        
        std::cout << "\nvar:" << std::endl;
        for( int i=0; i<_var_name.size(); i++)
        {
            std::cout << "var[" << i << "] = " << _var_name[i] << "    :: " << _var_description[i] << std::endl;
            
            symvar[i] = GiNaC::symbol( "var[" + itos(i) + "]");
            table[_var_name[i]] = symvar[i];
        }
        
        GiNaC::parser reader(table, true);
        GiNaC::ex symf;
        
        try
        {
            symf = reader(f);
        }
        catch (std::invalid_argument& err)
        {
            throw err;
        }
        
        
        // use 'symf' to generate c code strings for f, grad_f, hess_f
        
        _f = "";
        _grad.clear();
        _hess.clear();
        
        std::cout << "\n";
        if( _write_f)
        {
            std::ostringstream ss;
            ss << GiNaC::csrc_double << symf;    
            std::cout << " f :: " << ss.str() << std::endl;
            _f = ss.str();
        }
        
        std::cout << "\n";
        if( _write_grad)
        {
            for( int i=0; i<_var_name.size(); i++)
            {
                std::ostringstream ss;
                ss << GiNaC::csrc_double << symf.diff(symvar[i]);
                std::cout << " grad " << i << " :: " << ss.str() << std::endl;
                _grad.push_back( ss.str());
            }
        }
        
        std::cout << "\n";
        if( _write_hess)
        {
            _hess.resize(_var_name.size());
            for( int i=0; i<_var_name.size(); i++)
            {
                for( int j=0; j<_var_name.size(); j++)
                {
                    std::ostringstream ss;
                    ss << GiNaC::csrc_double << symf.diff(symvar[i]).diff(symvar[j]);
                    std::cout << " hess " << i << " " << j << " :: " << ss.str() << std::endl;
                    _hess[i].push_back( ss.str());
                }
            }
        }
        
        std::cout << " ------------------------------- \n";
        
        code_poly( sout);
        
    }
    
    /*
    void PFunctionWriter::code( const std::string &json_str,  std::ostream &sout)
    {
        std::cout << "PFunctionWriter::code( const std::string &json_str, std::ostream &sout) is undefined" << std::endl;
        exit(1);
    }
    */
    
    void PFunctionWriter::code( 
      const std::string &f, 
      const std::vector<std::string> &grad,
      const std::vector<std::vector< std::string> > &hess,
      std::ostream &sout)
    {
        _f = f;
        _grad = grad;
        _hess = hess;
        
        code_poly(sout);
        
    }
    
    void PFunctionWriter::write_basis_function(int I, const std::string &name, const std::string &f, std::ostream &sout) const
    {
        std::string PSimpleBaseTemplate;
        if( _template_intype) PSimpleBaseTemplate = "PSimpleBase< VarContainer, " + _outtype + ">";
        else PSimpleBaseTemplate = "PSimpleBase< " + _intype + ", " + _outtype + ">";
        
        if( _template_intype) sout << indent(I) << "template< class VarContainer>\n";
        sout << indent(I) << "class " + name + " : public " + PSimpleBaseTemplate + "\n";
        sout << indent(I) << "{\n";
        I++;
        if( _template_intype) sout << indent(I) << _outtype + " eval( const VarContainer &var) const\n";
        else sout << indent(I) << _outtype + " eval( const " + _intype + " &var) const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return " + f + ";\n";
        I--;
        sout << indent(I) << "}\n\n";
        I--;
        sout << indent(I) << "public:\n\n";
        I++;
        
        sout << indent(I) << name + "()\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "this->_name = \"" + name + "\";\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << name + "* clone() const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return new " + name + "(*this);\n";
        I--;
        sout << indent(I) << "}\n";
        
        I--;
        sout << indent(I) << "};\n\n";
    }
    
    // use polymorphic basis functions
    void PFunctionWriter::code_poly( 
      std::ostream &sout) const
    {
        int I = 1;
        
        std::string PFuncBaseTemplate, PSimpleBaseTemplate, PSimpleFunctionTemplate, instring, instringtemp;
        if( _template_intype)
        {
            PFuncBaseTemplate = "PFuncBase< VarContainer, " + _outtype + ">";
            PSimpleBaseTemplate = "PSimpleBase< VarContainer, " + _outtype + ">";
            PSimpleFunctionTemplate = "PSimpleFunction< VarContainer, " + _outtype + ">";
            instring = "VarContainer";
            instringtemp = "<VarContainer>";
        }
        else
        {
            PFuncBaseTemplate = "PFuncBase< " + _intype + ", " + _outtype + ">";
            PSimpleBaseTemplate = "PSimpleBase< " + _intype + ", " + _outtype + ">";
            PSimpleFunctionTemplate = "PSimpleFunction< " + _intype + ", " + _outtype + ">";
            instring = _intype;
            instringtemp = "";
        }
        
        // write function classes for f, grad_f, hess_f
        if( _write_f)
        {
            write_basis_function(I, _name + "_f", _f, sout);
        }
        if( _write_grad)
        {
            for( int i=0; i<_var_name.size(); i++)
                write_basis_function(I, _name + "_grad_" + itos(i), _grad[i], sout);
        }
        if( _write_hess)
        {
            for( int i=0; i<_var_name.size(); i++)
            for( int j=0; j<_var_name.size(); j++)
                write_basis_function(I, _name + "_hess_" + itos(i) + "_" + itos(j), _hess[i][j], sout);
        }
        
        
        // write class header
        if( _template_intype) sout << indent(I) << "template<class VarContainer>\n";
        sout << indent(I) << "class " + _name + " : public " + PFuncBaseTemplate + "\n";
        sout << indent(I) << "{\n";
        
        
        // write members
        sout << indent(I) << "public:\n";
        I++;
        //sout << indent(I) << "using " + PFuncBaseTemplate + "::_name;\n";
        //sout << indent(I) << "using " + PFuncBaseTemplate + "::_var_name;\n";
        //sout << indent(I) << "using " + PFuncBaseTemplate + "::_var_description;\n";
        sout << indent(I) << "\n";
        sout << indent(I) << PSimpleBaseTemplate + " *_val;\n";
        sout << indent(I) << PSimpleBaseTemplate + " **_grad_val;\n";
        sout << indent(I) << PSimpleBaseTemplate + " ***_hess_val;\n";
        sout << indent(I) << "\n";
        
        
        // write constructor
        sout << indent(I) << _name + "()\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "construct();\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        // write copy constructor
        sout << indent(I) << _name + "(const " + _name + " &RHS )\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "construct();\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        // write assignment operator
        sout << indent(I) << _name + "& operator=(const " + _name + " &RHS )\n";
        sout << indent(I) << "{\n";
        I++;
        if( _write_f)
        {
            sout << indent(I) << "_val = RHS._val;\n";
        }
        if( _write_grad)
        {
            sout << indent(I) << "\n";
            for( int i=0; i<_var_name.size(); i++)
                sout << indent(I) << "_grad_val[" + itos(i) + "] = RHS._grad_val[" + itos(i) + "];\n";
        }
        if( _write_hess)
        {
            for( int i=0; i<_var_name.size(); i++)
            for( int j=0; j<_var_name.size(); j++)
                sout << indent(I) << "_hess_val[" + itos(i) + "][" + itos(j) + "] = RHS._hess_val[" + itos(i) + "][" + itos(j) + "];\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write destructor
        sout << indent(I) << "~" + _name + "()\n";
        sout << indent(I) << "{\n";
        I++;
        if( _write_f)
        {
            sout << indent(I) << "delete _val;\n\n";
        }
        if( _write_grad)
        {
            for( int i=0; i<_var_name.size(); i++)
                sout << indent(I) << "delete _grad_val[" + itos(i) + "];\n";
            sout << indent(I) << "delete [] _grad_val;\n\n";
        }
        if( _write_hess)
        {
            for( int i=0; i<_var_name.size(); i++)
            for( int j=0; j<_var_name.size(); j++)
                sout << indent(I) << "delete _hess_val[" + itos(i) + "][" + itos(j) + "];\n";
            for( int i=0; i<_var_name.size(); i++)
                sout << indent(I) << "delete [] _hess_val[" + itos(i) + "];\n";
            sout << indent(I) << "delete [] _hess_val;\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write clone
        sout << indent(I) << _name + instringtemp + "* clone() const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return new " + _name + instringtemp + "(*this);\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        // write simplefunction getter
        sout << indent(I) << PSimpleFunctionTemplate + " simplefunction() const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return " + PSimpleFunctionTemplate + "( *_val );\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << PSimpleFunctionTemplate + " grad_simplefunction(int di) const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return " + PSimpleFunctionTemplate + "( *_grad_val[di] );\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << PSimpleFunctionTemplate + " hess_simplefunction(int di, int dj) const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return " + PSimpleFunctionTemplate + "( *_hess_val[di][dj] );\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        
        
        // write operators
        sout << indent(I) << _outtype + " operator()(const " + instring + " &var)\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return (*_val)(var);\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << _outtype + " grad(const " + instring + " &var, int di)\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return (*_grad_val[di])(var);\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << _outtype + " hess(const " + instring + " &var, int di, int dj)\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return (*_hess_val[di][dj])(var);\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write evals
        sout << indent(I) << "void eval(const " + instring + " &var)\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "(*_val)(var);\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << "void eval_grad(const " + instring + " &var)\n";
        sout << indent(I) << "{\n";
        I++;
        for( int i=0; i<_var_name.size(); i++)
            sout << indent(I) << "(*_grad_val[" + itos(i) + "])(var);\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << "void eval_hess(const " + instring + " &var)\n";
        sout << indent(I) << "{\n";
        I++;
        for( int i=0; i<_var_name.size(); i++)
        for( int j=0; j<_var_name.size(); j++)
        sout << indent(I) << "(*_hess_val[" + itos(i) + "][" + itos(j) + "])(var);\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write getters
        sout << indent(I) << _outtype + " operator()() const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return (*_val)();\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << _outtype + " grad(int di) const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return (*_grad_val[di])();\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << _outtype + " hess(int di, int dj) const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return (*_hess_val[di][dj])();\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        // write construct
        I--;
        sout << indent(I) << "private:\n";
        I++;
        sout << indent(I) << "void construct()\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "this->_name = \"" + _name + "\";\n";
        sout << indent(I) << "this->_var_name.clear();\n";
        for( int i=0; i<_var_name.size(); i++)
            sout << indent(I) << "this->_var_name.push_back(\"" + _var_name[i] + "\");\n";
        sout << indent(I) << "this->_var_description.clear();\n";
        for( int i=0; i<_var_description.size(); i++)
            sout << indent(I) << "this->_var_description.push_back(\"" + _var_description[i] + "\");\n";
        if( _write_f)
        {
            sout << indent(I) << "\n";
            sout << indent(I) << "_val = new " + _name + "_f" + instringtemp + "();\n";
        }
        if( _write_grad)
        {
            sout << indent(I) << "\n";
            sout << indent(I) << "_grad_val = new " + PSimpleBaseTemplate + "*[" + itos(_var_name.size()) + "];\n";
            for( int i=0; i<_var_name.size(); i++)
                sout << indent(I) << "_grad_val[" + itos(i) + "] = new " +_name + "_grad_" + itos(i) + instringtemp + "();\n";
        }
        if( _write_hess)
        {
            sout << indent(I) << "\n";
            sout << indent(I) << "_hess_val = new " + PSimpleBaseTemplate + "**[" + itos(_var_name.size()) + "];\n";
            for( int i=0; i<_var_name.size(); i++)
                sout << indent(I) << "_hess_val[" + itos(i) + "] = new " + PSimpleBaseTemplate + "*[" + itos(_var_name.size()) + "];\n";
            for( int i=0; i<_var_name.size(); i++)
            for( int j=0; j<_var_name.size(); j++)
                sout << indent(I) << "_hess_val[" + itos(i) + "][" + itos(j) + "] = new " +_name + "_hess_" + itos(i) + "_" + itos(j) + instringtemp + "();\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // close class
        I--;
        sout << indent(I) << "};\n";
        
        
    }
    
    // use if/else statements
    void PFunctionWriter::code_ifelse( 
      std::ostream &sout) const
    {
        int I = 0;
        std::string PFuncBaseTemplate, PSimpleBaseTemplate, PSimpleFunctionTemplate, instring, instringtemp;
        if( _template_intype)
        {
            PFuncBaseTemplate = "PFuncBase< VarContainer, " + _outtype + ">";
            PSimpleBaseTemplate = "PSimpleBase< VarContainer, " + _outtype + ">";
            PSimpleFunctionTemplate = "PSimpleFunction< VarContainer, " + _outtype + ">";
            instring = "VarContainer";
            instringtemp = "<VarContainer>";
        }
        else
        {
            PFuncBaseTemplate = "PFuncBase< " + _intype + ", " + _outtype + ">";
            PSimpleBaseTemplate = "PSimpleBase< " + _intype + ", " + _outtype + ">";
            PSimpleFunctionTemplate = "PSimpleFunction< " + _intype + ", " + _outtype + ">";
            instring = _intype;
            instringtemp = "";
        }
        // write function classes for f, grad_f, hess_f
        if( _write_f)
        {
            write_basis_function(I, _name + "_f", _f, sout);
        }
        if( _write_grad)
        {
            for( int i=0; i<_var_name.size(); i++)
                write_basis_function(I, _name + "_grad_" + itos(i), _grad[i], sout);
        }
        if( _write_hess)
        {
            for( int i=0; i<_var_name.size(); i++)
            for( int j=0; j<_var_name.size(); j++)
                write_basis_function(I, _name + "_hess_" + itos(i) + "_" + itos(j), _hess[i][j], sout);
        }
        
        // write class header
        if( _template_intype) sout << indent(I) << "template<class VarContainer>\n";
        sout << indent(I) << "class " + _name + " : public " + PFuncBaseTemplate + "\n";
        sout << indent(I) << "{\n";
        
        
        // write members
        sout << indent(I) << "public:\n";
        I++;
        sout << indent(I) << "using " + PFuncBaseTemplate + "::_name;\n";
        sout << indent(I) << "using " + PFuncBaseTemplate + "::_var_name;\n";
        sout << indent(I) << "using " + PFuncBaseTemplate + "::_var_description;\n";
        sout << indent(I) << "\n";
        sout << indent(I) << _outtype + " _val;\n";
        sout << indent(I) << _outtype + " *_grad_val;\n";
        sout << indent(I) << _outtype + " **_hess_val;\n";
        sout << indent(I) << "\n";
        
        
        // write constructor
        sout << indent(I) << _name + "()\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "construct();\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        // write copy constructor
        sout << indent(I) << _name + "(const " + _name + " &RHS )\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "construct();\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        // write assignment operator
        sout << indent(I) << _name + "& operator=(const " + _name + " &RHS )\n";
        sout << indent(I) << "{\n";
        I++;
        if( _write_f)
        {
            sout << indent(I) << "_val = RHS._val;\n";
        }
        if( _write_grad)
        {
            sout << indent(I) << "\n";
            for( int i=0; i<_var_name.size(); i++)
                sout << indent(I) << "_grad_val[" + itos(i) + "] = RHS._grad_val[" + itos(i) + "];\n";
        }
        if( _write_hess)
        {
            sout << indent(I) << "\n";
            for( int i=0; i<_var_name.size(); i++)
            for( int j=0; j<_var_name.size(); j++)
                sout << indent(I) << "_hess_val[" + itos(i) + "][" + itos(j) + "] = RHS._hess_val[" + itos(i) + "][" + itos(j) + "];\n";
        }
        sout << indent(I) << "return *this;\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write destructor
        sout << indent(I) << "~" + _name + "()\n";
        sout << indent(I) << "{\n";
        I++;
        if( _write_grad)
        {
            sout << indent(I) << "delete [] _grad_val;\n\n";
        }
        if( _write_hess)
        {
            for( int i=0; i<_var_name.size(); i++)
                sout << indent(I) << "delete [] _hess_val[" + itos(i) + "];\n";
            sout << indent(I) << "delete [] _hess_val;\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write clone
        sout << indent(I) << _name + instringtemp + "* clone() const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return new " + _name + instringtemp + "(*this);\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write simplefunction getter
        sout << indent(I) << PSimpleFunctionTemplate + " simplefunction() const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return " + PSimpleFunctionTemplate + "(" + _name + "_f());\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << PSimpleFunctionTemplate + " grad_simplefunction(int di) const\n";
        sout << indent(I) << "{\n";
        I++;
        for( int i=0; i<_var_name.size(); i++)
        {
            if( i == 0)
                sout << indent(I) << "if( di == " << i << " )\n";
            else
                sout << indent(I) << "else if( di == " << i << " )\n";
            sout << indent(I) << "{\n";
            I++;
            sout << indent(I) << "return " + PSimpleFunctionTemplate + "(" + _name + "_grad_" + itos(i) + "());\n";
            I--;
            sout << indent(I) << "}\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << PSimpleFunctionTemplate + " hess_simplefunction(int di, int dj) const\n";
        sout << indent(I) << "{\n";
        I++;
        for( int i=0; i<_var_name.size(); i++)
        {
            if( i == 0)
                sout << indent(I) << "if( di == " << i << " )\n";
            else
                sout << indent(I) << "else if( di == " << i << " )\n";
            sout << indent(I) << "{\n";
            I++;
            for( int j=0; j<_var_name.size(); j++)
            {
                if( j == 0)
                    sout << indent(I) << "if( dj == " << j << " )\n";
                else
                    sout << indent(I) << "else if( dj == " << j << " )\n";
                sout << indent(I) << "{\n";
                I++;
                sout << indent(I) << "return " + PSimpleFunctionTemplate + "(" + _name + "_hess_" + itos(i) + "_" + itos(j) + "());\n";
                I--;
                sout << indent(I) << "}\n";
            }
            I--;
            sout << indent(I) << "}\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write operators
        sout << indent(I) << _outtype + " operator()(const " + instring + " &var)\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return _val = " + _f + ";\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << _outtype + " grad(const " + instring + " &var, int di)\n";
        sout << indent(I) << "{\n";
        I++;
        for( int i=0; i<_var_name.size(); i++)
        {
            if( i == 0)
                sout << indent(I) << "if( di == " << i << " )\n";
            else
                sout << indent(I) << "else if( di == " << i << " )\n";
            sout << indent(I) << "{\n";
            I++;
            sout << indent(I) << "return _grad_val[di] = " + _grad[i] + ";\n";
            I--;
            sout << indent(I) << "}\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << _outtype + " hess(const " + instring + " &var, int di, int dj)\n";
        sout << indent(I) << "{\n";
        I++;
        for( int i=0; i<_var_name.size(); i++)
        {
            if( i == 0)
                sout << indent(I) << "if( di == " << i << " )\n";
            else
                sout << indent(I) << "else if( di == " << i << " )\n";
            sout << indent(I) << "{\n";
            I++;
            for( int j=0; j<_var_name.size(); j++)
            {
                if( j == 0)
                    sout << indent(I) << "if( dj == " << j << " )\n";
                else
                    sout << indent(I) << "else if( dj == " << j << " )\n";
                sout << indent(I) << "{\n";
                I++;
                sout << indent(I) << "return _hess_val[di][dj] = " + _hess[i][j] + ";\n";
                I--;
                sout << indent(I) << "}\n";
            }
            I--;
            sout << indent(I) << "}\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write evals
        sout << indent(I) << "void operator()(const " + instring + " &var)\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "_val = " + _f + ";\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << "void grad(const " + instring + " &var)\n";
        sout << indent(I) << "{\n";
        I++;
        for( int i=0; i<_var_name.size(); i++)
            sout << indent(I) << "_grad_val[" + itos(i) + "] = " + _grad[i] + ";\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << "void hess(const " + instring + " &var)\n";
        sout << indent(I) << "{\n";
        I++;
        for( int i=0; i<_var_name.size(); i++)
        for( int j=0; j<_var_name.size(); j++)
        sout << indent(I) << "_hess_val[" + itos(i) + "][" + itos(j) + "] = " + _hess[i][j] + ";\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // write getters
        sout << indent(I) << _outtype + " operator()() const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return _val;\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << _outtype + " grad(int di) const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return _grad_val[di];\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        sout << indent(I) << _outtype + " hess(int di, int dj) const\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "return _hess_val[di][dj];\n";
        I--;
        sout << indent(I) << "}\n\n";
        
        // write construct
        I--;
        sout << indent(I) << "private:\n";
        I++;
        sout << indent(I) << "void construct()\n";
        sout << indent(I) << "{\n";
        I++;
        sout << indent(I) << "_name = \"" + _name + "\";\n";
        sout << indent(I) << "_var_name.clear();\n";
        for( int i=0; i<_var_name.size(); i++)
            sout << indent(I) << "_var_name.push_back(\"" + _var_name[i] + "\");\n";
        sout << indent(I) << "_var_description.clear();\n";
        for( int i=0; i<_var_description.size(); i++)
            sout << indent(I) << "_var_description.push_back(\"" + _var_description[i] + "\");\n";
        
        if( _write_grad)
        {
            sout << indent(I) << "\n";
            sout << indent(I) << "_grad_val = new " + _outtype + "[" + itos(_var_name.size()) + "];\n";
        }
        if( _write_hess)
        {
            sout << indent(I) << "\n";
            sout << indent(I) << "_hess_val = new " + _outtype + "*[" + itos(_var_name.size()) + "];\n";
            for( int i=0; i<_var_name.size(); i++)
                sout << indent(I) << "_hess_val[" + itos(i) + "] = new " + _outtype + "[" + itos(_var_name.size()) + "];\n";
        }
        I--;
        sout << indent(I) << "}\n\n";
        
        
        // close class
        I--;
        sout << indent(I) << "};\n";
        
    }
    
    /*
    void PFunctionWriter::autodiff( std::ostream &sout)
    {
        std::cout << "PFunctionWriter::autodiff( std::ostream &sout) is undefined" << std::endl;
        exit(1);
    }
    */
    
    /*
    void PFunctionWriter::series( std::ostream &sout)
    {
        std::cout << "PFunctionWriter::series( std::ostream &sout) is undefined" << std::endl;
        exit(1);
    }
    */
    
    //private:
    
    std::string PFunctionWriter::indent(int step) const
    {
        std::string tmp = "";
        for( int i=0; i<step; i++)
            tmp += _basic_indent;
        return tmp;
    }
    
    void PFunctionWriter::head( std::ostream &sout) const
    {
        int I = 0;
        
        // write date this was written
        
        sout << indent(I) << "// created: " << now() << "\n";
        sout << indent(I) << "// version: " << IntegrationTools_version_id() << "\n";
        sout << indent(I) << "// url: " << IntegrationTools_repo_url() << "\n";
        sout << indent(I) << "// commit: " << IntegrationTools_commit_id() << "\n\n";
        
        // write include guards & namespace
        sout << indent(I) << "#ifndef " + _name + "_HH\n"; 
        sout << indent(I) << "#define " + _name + "_HH\n\n"; 
        sout << indent(I) << "#include <cmath>\n";
        sout << indent(I) << "#include <cstdlib>\n";
        sout << indent(I) << "#include \"PFunction.hh\"\n\n";
        sout << indent(I) << "namespace PRISMS\n"; 
        sout << indent(I) << "{\n";
        I++;
        
    }
    
    void PFunctionWriter::foot( std::ostream &sout) const
    {
        // close namespace
        sout << indent(0) << "}\n";
        
        // finish
        sout << indent(0) << "#endif\n";
        
    }

}


#endif