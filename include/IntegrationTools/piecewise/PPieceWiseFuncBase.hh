
#ifndef PPieceWiseFuncBase_HH
#define PPieceWiseFuncBase_HH

#include<iostream>
#include<vector>
#include<stdexcept>

#include "../pfunction/PFuncBase.hh"
#include "./SimplePiece.hh"
#include "./PPieceWiseSimpleBase.hh"
#include "./Piece.hh"

namespace PRISMS
{   
    
    /// Class to define a PieceWise Function
    /// 
    ///   Contains a vector of 'Piece'. Throws a domain_error if it 
    ///   is evaluated outside of the valid domain of any piece.
    ///
    template< class VarContainer, class OutType>
    class PPieceWiseFuncBase : public PFuncBase<VarContainer, OutType> 
    {
        public:
        
        int _curr_piece;
        std::vector<Piece<VarContainer, OutType> > _piece;
        
        PPieceWiseFuncBase() {}
        
        PPieceWiseFuncBase( const std::vector<Piece<VarContainer, OutType> > &piece)
        {
            _piece = piece;
        }
        
        bool in_piece( const VarContainer &var) const
        {
            for( int i=0; i<_piece.size(); i++)
            {
                if( _piece[i].in_piece(var) )
                    return true;
            }
            return false;
        }
        
        int piece(const VarContainer &var)
        {
            for( int i=0; i<_piece.size(); i++)
            {
                if( _piece[i].in_piece(var))
                    return _curr_piece = i;
            }
            
            throw std::domain_error("PPieceWiseFuncBase: Not in any piece");
        }
        
        virtual PPieceWiseFuncBase<VarContainer, OutType> *clone() const
        {
            return new PPieceWiseFuncBase<VarContainer, OutType>(*this);
        }
        
        virtual PSimpleFunction<VarContainer, OutType> simplefunction() const
        {
            std::vector<SimplePiece<VarContainer, OutType> > piece;
            
            for( int i=0; i<_piece.size(); i++)
            {
                piece.push_back( _piece[i].simplepiece() );
            }
            
            return PSimpleFunction<VarContainer, OutType>( PPieceWiseSimpleBase<VarContainer, OutType>(piece) );
        }
        
        virtual PSimpleFunction<VarContainer, OutType> grad_simplefunction(int di) const
        {
            std::vector<SimplePiece<VarContainer, OutType> > piece;
            
            for( int i=0; i<_piece.size(); i++)
            {
                piece.push_back( _piece[i].grad_simplepiece(di));
            }
            
            return PSimpleFunction<VarContainer, OutType>( PPieceWiseSimpleBase<VarContainer, OutType>(piece));
        }
        
        virtual PSimpleFunction<VarContainer, OutType> hess_simplefunction(int di, int dj) const
        {
            std::vector<SimplePiece<VarContainer, OutType> > piece;
            
            for( int i=0; i<_piece.size(); i++)
            {
                piece.push_back( _piece[i].hess_simplepiece(di,dj));
            }
            
            return PSimpleFunction<VarContainer, OutType>( PPieceWiseSimpleBase<VarContainer, OutType>(piece) );
        }

        // ----------------------------------------------------------
        // Use these functions if you want to evaluate a single value
        
        virtual OutType operator()(const VarContainer &var)
        {
            return _piece[piece(var)](var);
        }
        virtual OutType grad(const VarContainer &var, int di)
        {
            return _piece[piece(var)].grad(var, di);
        }
        virtual OutType hess(const VarContainer &var, int di, int dj)
        {
            return _piece[piece(var)].hess(var, di, dj);
        }

        // ----------------------------------------------------------
        // Use these functions to evaluate several values, then use 'get' methods to access results
        virtual void eval(const VarContainer &var)
        {
            _piece[piece(var)].eval(var);
        }
        virtual void eval_grad(const VarContainer &var)
        {
            _piece[piece(var)].eval_grad(var);
        }
        virtual void eval_hess(const VarContainer &var)
        {
            _piece[piece(var)].eval_hess(var);
        }
        
        /// These don't recheck the domain
        virtual OutType operator()() const
        {
            return _piece[_curr_piece]();
        }
        virtual OutType grad(int di) const
        {
            return _piece[_curr_piece].grad(di);
        }
        virtual OutType hess(int di, int dj) const
        {
            return _piece[_curr_piece].hess(di, dj);
        }
    };

}


#endif