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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <ne_locks.h>
#include <ne_uri.h>
#include "rtl/ustring.hxx"
#include "osl/time.h"
#include "osl/thread.hxx"
#include "NeonSession.hxx"
#include "NeonLockStore.hxx"

using namespace webdav_ucp;

namespace webdav_ucp {

class TickerThread : public osl::Thread
{
    bool m_bFinish;
    NeonLockStore & m_rLockStore;

public:

    TickerThread( NeonLockStore & rLockStore )
    : osl::Thread(), m_bFinish( false ), m_rLockStore( rLockStore ) {}

    void finish() { m_bFinish = true; }

protected:

    virtual void SAL_CALL run();
};

} // namespace webdav_ucp

// -------------------------------------------------------------------
void TickerThread::run()
{
    OSL_TRACE( "TickerThread: start." );

    // we have to go through the loop more often to be able to finish ~quickly
    const int nNth = 25;

    int nCount = nNth;
    while ( !m_bFinish )
    {
        if ( nCount-- <= 0 )
        {
            m_rLockStore.refreshLocks();
            nCount = nNth;
        }

        TimeValue aTV;
        aTV.Seconds = 0;
        aTV.Nanosec = 1000000000 / nNth;
        wait( aTV );
    }

    OSL_TRACE( "TickerThread: stop." );
}

// -------------------------------------------------------------------
NeonLockStore::NeonLockStore()
    : m_pNeonLockStore( ne_lockstore_create() ),
      m_pTickerThread( 0 )
{
    OSL_ENSURE( m_pNeonLockStore, "Unable to create neon lock store!" );
}

// -------------------------------------------------------------------
NeonLockStore::~NeonLockStore()
{
    stopTicker();

    // release active locks, if any.
    OSL_ENSURE( m_aLockInfoMap.size() == 0,
                "NeonLockStore::~NeonLockStore - Releasing active locks!" );

    LockInfoMap::const_iterator it( m_aLockInfoMap.begin() );
    const LockInfoMap::const_iterator end( m_aLockInfoMap.end() );
    while ( it != end )
    {
        NeonLock * pLock = (*it).first;
        (*it).second.xSession->UNLOCK( pLock );

        ne_lockstore_remove( m_pNeonLockStore, pLock );
        ne_lock_destroy( pLock );

        ++it;
    }

    ne_lockstore_destroy( m_pNeonLockStore );
}

// -------------------------------------------------------------------
void NeonLockStore::startTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pTickerThread )
    {
        m_pTickerThread = new TickerThread( *this );
        m_pTickerThread->create();
    }
}

// -------------------------------------------------------------------
void NeonLockStore::stopTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pTickerThread )
    {
        m_pTickerThread->finish();
        m_pTickerThread->join();
        delete m_pTickerThread;
        m_pTickerThread = 0;
    }
}

// -------------------------------------------------------------------
void NeonLockStore::registerSession( HttpSession * pHttpSession )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_lockstore_register( m_pNeonLockStore, pHttpSession );
}

// -------------------------------------------------------------------
NeonLock * NeonLockStore::findByUri( rtl::OUString const & rUri )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_uri aUri;
    ne_uri_parse( rtl::OUStringToOString(
        rUri, RTL_TEXTENCODING_UTF8 ).getStr(), &aUri );
    return ne_lockstore_findbyuri( m_pNeonLockStore, &aUri );
}

// -------------------------------------------------------------------
void NeonLockStore::addLock( NeonLock * pLock,
                             rtl::Reference< NeonSession > const & xSession,
                             sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_lockstore_add( m_pNeonLockStore, pLock );
    m_aLockInfoMap[ pLock ]
        = LockInfo( xSession, nLastChanceToSendRefreshRequest );

    startTicker();
}

// -------------------------------------------------------------------
void NeonLockStore::updateLock( NeonLock * pLock,
                                sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::iterator it( m_aLockInfoMap.find( pLock ) );
    OSL_ENSURE( it != m_aLockInfoMap.end(),
                "NeonLockStore::updateLock: lock not found!" );

    if ( it != m_aLockInfoMap.end() )
    {
        (*it).second.nLastChanceToSendRefreshRequest
            = nLastChanceToSendRefreshRequest;
    }
}

// -------------------------------------------------------------------
void NeonLockStore::removeLock( NeonLock * pLock )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aLockInfoMap.erase( pLock );
    ne_lockstore_remove( m_pNeonLockStore, pLock );

    if ( m_aLockInfoMap.size() == 0 )
        stopTicker();
}

// -------------------------------------------------------------------
void NeonLockStore::refreshLocks()
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::iterator it( m_aLockInfoMap.begin() );
    const LockInfoMap::const_iterator end( m_aLockInfoMap.end() );
    while ( it != end )
    {
        LockInfo & rInfo = (*it).second;
        if ( rInfo.nLastChanceToSendRefreshRequest != -1 )
        {
            // 30 seconds or less remaining until lock expires?
            TimeValue t1;
            osl_getSystemTime( &t1 );
            if ( rInfo.nLastChanceToSendRefreshRequest - 30
                     <= sal_Int32( t1.Seconds ) )
            {
                // refresh the lock.
                sal_Int32 nlastChanceToSendRefreshRequest = -1;
                if ( rInfo.xSession->LOCK(
                         (*it).first,
                         /* out param */ nlastChanceToSendRefreshRequest ) )
                {
                    rInfo.nLastChanceToSendRefreshRequest
                        = nlastChanceToSendRefreshRequest;
                }
                else
                {
                    // refresh failed. stop auto-refresh.
                    rInfo.nLastChanceToSendRefreshRequest = -1;
                }
            }
        }
        ++it;
    }
}
