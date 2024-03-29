// This file is part of libigl, a simple c++ geometry processing library.
//
// Copyright (C) 2020 Oded Stein <oded.stein@columbia.edu>
//
// This Source Code Form is subject to the terms of the Mozilla Public License
// v. 2.0. If a copy of the MPL was not distributed with this file, You can
// obtain one at http://mozilla.org/MPL/2.0/.
#include "cr_vector_mass.h"

#include <vector>

#include "orient_halfedges.h"

#include "doublearea.h"
#include "squared_edge_lengths.h"


template <typename DerivedV, typename DerivedF, typename DerivedE, typename ScalarM>
IGL_INLINE void
igl::cr_vector_mass(
  const Eigen::MatrixBase<DerivedV>& V,
  const Eigen::MatrixBase<DerivedF>& F,
  const Eigen::MatrixBase<DerivedE>& E,
  Eigen::SparseMatrix<ScalarM>& M)
{
  Eigen::Matrix<typename DerivedV::Scalar, Eigen::Dynamic, Eigen::Dynamic>
  dblA;
  doublearea(V,F,dblA);
  cr_vector_mass_intrinsic(F, dblA, E, M);
}


template <
  typename DerivedV, 
  typename DerivedF, 
  typename DerivedE, 
  typename ScalarM>
IGL_INLINE void
igl::cr_vector_mass(
  const Eigen::MatrixBase<DerivedV>& V,
  const Eigen::MatrixBase<DerivedF>& F,
  Eigen::PlainObjectBase<DerivedE>& E,
  Eigen::SparseMatrix<ScalarM>& M)
{
  if(E.rows()!=F.rows() || E.cols()!=F.cols())
  {
    DerivedE oE;
    orient_halfedges(F, E, oE);
  }

  const Eigen::PlainObjectBase<DerivedE>& cE = E;
  cr_vector_mass(V, F, cE, M);
}


template <typename DerivedF, typename DeriveddA,
typename DerivedE, typename ScalarM>
IGL_INLINE void
igl::cr_vector_mass_intrinsic(
 const Eigen::MatrixBase<DerivedF>& F,
 const Eigen::MatrixBase<DeriveddA>& dA,
 const Eigen::MatrixBase<DerivedE>& E,
 Eigen::SparseMatrix<ScalarM>& M)
{
  assert(F.cols()==3 && "Faces have three vertices");
  assert(E.rows()==F.rows() && E.cols()==F.cols() && "Wrong dimension in edge vectors");

  const Eigen::Index m = F.rows();
  const typename DerivedE::Scalar nE = E.maxCoeff() + 1;

  std::vector<Eigen::Triplet<ScalarM> > tripletList;
  tripletList.reserve(2*3*m);
  for(Eigen::Index f=0; f<m; ++f) {
    for(int e=0; e<3; ++e) {
      //Scaled
      const ScalarM entry = dA(f) / 6;
      tripletList.emplace_back(E(f,e), E(f,e), entry);
      tripletList.emplace_back(E(f,e)+nE, E(f,e)+nE, entry);
    }
  }
  M.resize(2*nE, 2*nE);
  M.setFromTriplets(tripletList.begin(), tripletList.end());
}


#ifdef IGL_STATIC_LIBRARY
// Explicit template instantiation
// generated by autoexplicit.sh
template void igl::cr_vector_mass<Eigen::Matrix<double, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, double>(Eigen::MatrixBase<Eigen::Matrix<double, -1, -1, 0, -1, -1>> const&, Eigen::MatrixBase<Eigen::Matrix<int, -1, -1, 0, -1, -1>> const&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, -1, 0, -1, -1>>&, Eigen::SparseMatrix<double, 0, int>&);
#endif
