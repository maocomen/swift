//===----- IDETypeCheckingRequests.h - IDE type-check Requests --*- C++ -*-===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2019 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
//  This file defines IDE type checking request using the evaluator model.
//  The file needs to exist in sema because it needs internal implementation
//  of the type checker to fulfill some requests
//
//===----------------------------------------------------------------------===//
#ifndef SWIFT_IDE_TYPE_CHECKING_REQUESTS_H
#define SWIFT_IDE_TYPE_CHECKING_REQUESTS_H

#include "swift/AST/ASTTypeIDs.h"
#include "swift/AST/Evaluator.h"
#include "swift/AST/SimpleRequest.h"
#include "swift/AST/TypeCheckRequests.h"

namespace swift {
//----------------------------------------------------------------------------//
// Decl applicability checking
//----------------------------------------------------------------------------//
struct DeclApplicabilityOwner {
  const DeclContext *DC;
  const Type Ty;
  const Decl *ExtensionOrMember;

  DeclApplicabilityOwner(const DeclContext *DC, Type Ty, const ExtensionDecl *ED):
    DC(DC), Ty(Ty), ExtensionOrMember(ED) {}
  DeclApplicabilityOwner(const DeclContext *DC, Type Ty, const ValueDecl *VD):
    DC(DC), Ty(Ty), ExtensionOrMember(VD) {}

  friend llvm::hash_code hash_value(const DeclApplicabilityOwner &CI) {
    return hash_combine(hash_value(CI.Ty.getPointer()),
                        hash_value(CI.ExtensionOrMember));
  }

  friend bool operator==(const DeclApplicabilityOwner &lhs,
                         const DeclApplicabilityOwner &rhs) {
    return lhs.Ty.getPointer() == rhs.Ty.getPointer() &&
      lhs.ExtensionOrMember == rhs.ExtensionOrMember;
  }

  friend bool operator!=(const DeclApplicabilityOwner &lhs,
                         const DeclApplicabilityOwner &rhs) {
    return !(lhs == rhs);
  }

  friend void simple_display(llvm::raw_ostream &out,
                             const DeclApplicabilityOwner &owner) {
    out << "Checking if ";
    simple_display(out, owner.ExtensionOrMember);
    out << " is applicable for ";
    simple_display(out, owner.Ty);
  }
};

class IsDeclApplicableRequest:
    public SimpleRequest<IsDeclApplicableRequest,
                         bool(DeclApplicabilityOwner),
                         CacheKind::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  llvm::Expected<bool> evaluate(Evaluator &evaluator,
                                DeclApplicabilityOwner Owner) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

//----------------------------------------------------------------------------//
// Type relation checking
//----------------------------------------------------------------------------//
enum class TypeRelation: uint8_t {
  EqualTo,
  PossiblyEqualTo,
  ConvertTo,
};

struct TypePair {
  Type FirstTy;
  Type SecondTy;
  TypePair(Type FirstTy, Type SecondTy): FirstTy(FirstTy), SecondTy(SecondTy) {}
  TypePair(): TypePair(Type(), Type()) {}
  friend llvm::hash_code hash_value(const TypePair &TI) {
    return hash_combine(hash_value(TI.FirstTy.getPointer()),
                        hash_value(TI.SecondTy.getPointer()));
  }

  friend bool operator==(const TypePair &lhs,
                         const TypePair &rhs) {
    return lhs.FirstTy.getPointer() == rhs.FirstTy.getPointer() &&
      lhs.SecondTy.getPointer() == rhs.SecondTy.getPointer();
  }

  friend bool operator!=(const TypePair &lhs,
                         const TypePair &rhs) {
    return !(lhs == rhs);
  }

  friend void simple_display(llvm::raw_ostream &out,
                             const TypePair &owner) {
    out << "<";
    simple_display(out, owner.FirstTy);
    out << ", ";
    simple_display(out, owner.SecondTy);
    out << ">";
  }
};

struct TypeRelationCheckInput {
  DeclContext *DC;
  TypePair Pair;
  TypeRelation Relation;
  bool OpenArchetypes;

  TypeRelationCheckInput(DeclContext *DC, Type FirstType, Type SecondType,
                         TypeRelation Relation, bool OpenArchetypes = true):
    DC(DC), Pair(FirstType, SecondType), Relation(Relation),
    OpenArchetypes(OpenArchetypes) {}

  friend llvm::hash_code hash_value(const TypeRelationCheckInput &TI) {
    return hash_combine(hash_value(TI.Pair),
                        hash_value(TI.Relation),
                        hash_value(TI.OpenArchetypes));
  }

  friend bool operator==(const TypeRelationCheckInput &lhs,
                         const TypeRelationCheckInput &rhs) {
    return lhs.Pair == rhs.Pair && lhs.Relation == rhs.Relation &&
      lhs.OpenArchetypes == rhs.OpenArchetypes;
  }

  friend bool operator!=(const TypeRelationCheckInput &lhs,
                         const TypeRelationCheckInput &rhs) {
    return !(lhs == rhs);
  }

  friend void simple_display(llvm::raw_ostream &out,
                               const TypeRelationCheckInput &owner) {
    out << "Check if ";
    simple_display(out, owner.Pair);
    out << " is ";
    switch(owner.Relation) {
#define CASE(NAME) case TypeRelation::NAME: out << #NAME << " "; break;
    CASE(EqualTo)
    CASE(PossiblyEqualTo)
    CASE(ConvertTo)
#undef CASE
    }
  }
};

class TypeRelationCheckRequest:
    public SimpleRequest<TypeRelationCheckRequest,
                         bool(TypeRelationCheckInput),
                         CacheKind::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  llvm::Expected<bool> evaluate(Evaluator &evaluator,
                                TypeRelationCheckInput Owner) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

//----------------------------------------------------------------------------//
// RootAndResultTypeOfKeypathDynamicMemberRequest
//----------------------------------------------------------------------------//
class RootAndResultTypeOfKeypathDynamicMemberRequest:
    public SimpleRequest<RootAndResultTypeOfKeypathDynamicMemberRequest,
                         TypePair(SubscriptDecl*),
                         CacheKind::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  llvm::Expected<TypePair> evaluate(Evaluator &evaluator, SubscriptDecl* SD) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

class RootTypeOfKeypathDynamicMemberRequest:
    public SimpleRequest<RootTypeOfKeypathDynamicMemberRequest,
                         Type(SubscriptDecl*),
                         /*Cached in the request above*/CacheKind::Uncached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  llvm::Expected<Type> evaluate(Evaluator &evaluator, SubscriptDecl* SD) const {
    return evaluateOrDefault(SD->getASTContext().evaluator,
      RootAndResultTypeOfKeypathDynamicMemberRequest{SD}, TypePair()).
        FirstTy;
  }

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

//----------------------------------------------------------------------------//
// HasDynamicMemberLookupAttributeRequest
//----------------------------------------------------------------------------//
class HasDynamicMemberLookupAttributeRequest:
    public SimpleRequest<HasDynamicMemberLookupAttributeRequest,
                         bool(TypeBase*),
                         CacheKind::Cached> {
public:
  using SimpleRequest::SimpleRequest;

private:
  friend SimpleRequest;

  // Evaluation.
  llvm::Expected<bool> evaluate(Evaluator &evaluator, TypeBase *ty) const;

public:
  // Caching
  bool isCached() const { return true; }
  // Source location
  SourceLoc getNearestLoc() const { return SourceLoc(); };
};

/// The zone number for the IDE.
#define SWIFT_TYPEID_ZONE IDETypeChecking
#define SWIFT_TYPEID_HEADER "swift/Sema/IDETypeCheckingRequestIDZone.def"
#include "swift/Basic/DefineTypeIDZone.h"
#undef SWIFT_TYPEID_ZONE
#undef SWIFT_TYPEID_HEADER

// Set up reporting of evaluated requests.
#define SWIFT_REQUEST(Zone, RequestType)                         \
template<>                                                       \
inline void reportEvaluatedRequest(UnifiedStatsReporter &stats,  \
                            const RequestType &request) {        \
  ++stats.getFrontendCounters().RequestType;                     \
}
#include "swift/Sema/IDETypeCheckingRequestIDZone.def"
#undef SWIFT_REQUEST

} // end namespace swift

#endif // SWIFT_IDE_REQUESTS_H
