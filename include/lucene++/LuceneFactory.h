/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENEFACTORY_H
#define LUCENEFACTORY_H

#include <memory>

namespace Lucene {

template <typename T, Args ... args>
std::shared_ptr<T> newInstance(Args && ... args) {
    return std::make_shared<T>(std::forward<Args>(args) ... ); 
}


template <typename T, Args ... args>
std::shared_ptr<T> newLucene(Args && ... args){
    std::shared_ptr<T> instance = std::make_shared<T>(std::forward<Args>(args)...);
    instance->initialize();
    return instance;
}

} // namespace Lucene

#endif
