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



#include <rtl/memory.h>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <string.h>

#include "gio_outputstream.hxx"
#include "gio_content.hxx"

using namespace com::sun::star;

namespace gio
{

OutputStream::OutputStream(GFileOutputStream *pStream) : Seekable(G_SEEKABLE(pStream)), mpStream(pStream)
{
    if (!mpStream)
        throw io::NotConnectedException();
}

OutputStream::~OutputStream( void )
{
    closeOutput();
}

void SAL_CALL OutputStream::writeBytes( const com::sun::star::uno::Sequence< sal_Int8 >& rData )
    throw( io::NotConnectedException, io::BufferSizeExceededException,
           io::IOException, uno::RuntimeException)
{
    if (!mpStream)
        throw io::NotConnectedException();

    GError *pError=NULL;
    if (!g_output_stream_write_all(G_OUTPUT_STREAM(mpStream), rData.getConstArray(), rData.getLength(), NULL, NULL, &pError))
        convertToException(pError, static_cast< cppu::OWeakObject * >(this));
}

void SAL_CALL OutputStream::flush( void )
    throw( io::NotConnectedException, io::BufferSizeExceededException,
           io::IOException, uno::RuntimeException )
{
    if (!mpStream)
        throw io::NotConnectedException();

    GError *pError=NULL;
    if (!g_output_stream_flush(G_OUTPUT_STREAM(mpStream), NULL, &pError))
        convertToException(pError, static_cast< cppu::OWeakObject * >(this));
}

void SAL_CALL OutputStream::closeOutput( void )
    throw( io::NotConnectedException, io::IOException,
           uno::RuntimeException )
{
    if (mpStream)
        g_output_stream_close(G_OUTPUT_STREAM(mpStream), NULL, NULL);
}

uno::Any OutputStream::queryInterface( const uno::Type &type ) throw( uno::RuntimeException )
{
    uno::Any aRet = ::cppu::queryInterface ( type,
        static_cast< XOutputStream * >( this ) );

    return aRet.hasValue() ? aRet : Seekable::queryInterface( type );
}

}
