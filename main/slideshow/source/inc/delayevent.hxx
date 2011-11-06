/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/


#ifndef INCLUDED_SLIDESHOW_DELAYEVENT_HXX
#define INCLUDED_SLIDESHOW_DELAYEVENT_HXX

#include <boost/function.hpp>

#include "event.hxx"
#include "debug.hxx"
#include <boost/noncopyable.hpp>

namespace slideshow {
namespace internal {

/** Event, which delays the functor call the given amount of time
 */
class Delay : public Event, private ::boost::noncopyable
{
public:
    typedef ::boost::function0<void> FunctorT;
    
    template <typename FuncT>
        Delay( FuncT const& func,
               double nTimeout
#if OSL_DEBUG_LEVEL > 1
            ,  const ::rtl::OUString& rsDescription
            ) : Event(rsDescription),
#else
            ) :
#endif
            mnTimeout(nTimeout), maFunc(func), mbWasFired(false) {}
    
    Delay( const boost::function0<void>& func,
           double nTimeout
#if OSL_DEBUG_LEVEL > 1
        , const ::rtl::OUString& rsDescription
        ) : Event(rsDescription),
#else
        ) :
#endif
        mnTimeout(nTimeout),
        maFunc(func),
        mbWasFired(false) {}
    
    // Event:
    virtual bool fire();
    virtual bool isCharged() const;
    virtual double getActivationTime( double nCurrentTime ) const;
    // Disposable:
    virtual void dispose();
    
private:
    double const mnTimeout;  
    FunctorT maFunc;
    bool mbWasFired;
};

#if OSL_DEBUG_LEVEL <= 1

/** Generate delay event
    
    @param func
    Functor to call when the event fires.
    
    @param nTimeout
    Timeout in seconds, to wait until functor is called.
    
    @return generated delay event
*/
template <typename FuncT>
inline EventSharedPtr makeDelay_( FuncT const& func, double nTimeout )
{
    return EventSharedPtr( new Delay( func, nTimeout ) );
}

/** Generate immediate event
    
    @param func
    Functor to call when the event fires.
    
    @return generated immediate event.
*/
template <typename FuncT>
inline EventSharedPtr makeEvent_( FuncT const& func )
{
    return EventSharedPtr( new Delay( func, 0.0 ) );
}


// Strip away description.
#define makeDelay(f, t, d) makeDelay_(f, t)
#define makeEvent(f, d) makeEvent_(f)

#else // OSL_DEBUG_LEVEL > 1

class Delay_ : public Delay {
public:
    template <typename FuncT>
    Delay_( FuncT const& func, double nTimeout,
        char const* from_function, char const* from_file, int from_line,
        const ::rtl::OUString& rsDescription)
        : Delay(func, nTimeout, rsDescription),
          FROM_FUNCTION(from_function),
          FROM_FILE(from_file), FROM_LINE(from_line) {}
    
    char const* const FROM_FUNCTION;
    char const* const FROM_FILE;
    int const FROM_LINE;
};

template <typename FuncT>
inline EventSharedPtr makeDelay_(
    FuncT const& func, double nTimeout,
    char const* from_function, char const* from_file, int from_line,
    const ::rtl::OUString& rsDescription)
{
    return EventSharedPtr( new Delay_( func, nTimeout,
            from_function, from_file, from_line, rsDescription) );
}

#define makeDelay(f, t, d) makeDelay_(f, t,                   \
        BOOST_CURRENT_FUNCTION, __FILE__, __LINE__,           \
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(d)))
#define makeEvent(f, d) makeDelay_(f, 0.0,                  \
        BOOST_CURRENT_FUNCTION, __FILE__, __LINE__,         \
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(d)))

#endif // OSL_DEBUG_LEVEL <= 1

} // namespace internal
} // namespace presentation

#endif /* INCLUDED_SLIDESHOW_DELAYEVENT_HXX */
