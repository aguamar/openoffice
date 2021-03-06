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
#include "precompiled_unotools.hxx"
#include <unotools/collatorwrapper.hxx>
#include <tools/debug.hxx>

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace ::com::sun::star;

CollatorWrapper::CollatorWrapper (
		const uno::Reference< lang::XMultiServiceFactory > &xServiceFactory)
	: mxServiceFactory (xServiceFactory)
{
	::rtl::OUString aService (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.Collator"));

	if (mxServiceFactory.is())
	{
		try
		{
			mxInternationalCollator = uno::Reference< i18n::XCollator > (
				mxServiceFactory->createInstance(aService), uno::UNO_QUERY);
		}
		catch (uno::Exception& rException)
		{
            (void)rException;
			DBG_ERRORFILE ("CollatorWrapper: failed to create instance");
		}
	}
	else
	{
		::rtl::OUString aLibrary (RTL_CONSTASCII_USTRINGPARAM(LLCF_LIBNAME("i18n")));

		try
		{
			uno::Reference< uno::XInterface > xInstance =
				::comphelper::getComponentInstance (aLibrary, aService);

			if (xInstance.is())
			{
				uno::Any xInterface = xInstance->queryInterface (
					::getCppuType((const uno::Reference< i18n::XCollator >*)0) );
				xInterface >>= mxInternationalCollator;
			}
		}
		catch (uno::Exception& rException)
		{
            (void)rException;
			DBG_ERRORFILE ("CollatorWrapper: failed to get component instance!");
		}
	}

	DBG_ASSERT (mxInternationalCollator.is(), "CollatorWrapper: no i18n collator");
}

CollatorWrapper::~CollatorWrapper()
{
}

sal_Int32
CollatorWrapper::compareSubstring (
		const ::rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const ::rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) const
{
	try
	{
		if (mxInternationalCollator.is())
			return mxInternationalCollator->compareSubstring (
														s1, off1, len1, s2, off2, len2);
	}
	catch (uno::RuntimeException& rRuntimeException)
	{
        (void)rRuntimeException;
		DBG_ERRORFILE ("CollatorWrapper: compareSubstring failed");
	}

	return 0;
}

sal_Int32
CollatorWrapper::compareString (const ::rtl::OUString& s1, const ::rtl::OUString& s2) const
{
	try
	{
		if (mxInternationalCollator.is())
			return mxInternationalCollator->compareString (s1, s2);
	}
	catch (uno::RuntimeException& rRuntimeException)
	{
        (void)rRuntimeException;
		DBG_ERRORFILE ("CollatorWrapper: compareString failed");
	}

	return 0;
}

uno::Sequence< ::rtl::OUString >
CollatorWrapper::listCollatorAlgorithms (const lang::Locale& rLocale) const
{
	try
	{
		if (mxInternationalCollator.is())
			return mxInternationalCollator->listCollatorAlgorithms (rLocale);
	}
	catch (uno::RuntimeException& rRuntimeException)
	{
        (void)rRuntimeException;
		DBG_ERRORFILE ("CollatorWrapper: listCollatorAlgorithms failed");
	}

	return uno::Sequence< ::rtl::OUString > ();
}

uno::Sequence< sal_Int32 >
CollatorWrapper::listCollatorOptions (const ::rtl::OUString& rAlgorithm) const
{
	try
	{
		if (mxInternationalCollator.is())
			return mxInternationalCollator->listCollatorOptions (rAlgorithm);
	}
	catch (uno::RuntimeException& rRuntimeException)
	{
        (void)rRuntimeException;
		DBG_ERRORFILE ("CollatorWrapper: listCollatorOptions failed");
	}

	return uno::Sequence< sal_Int32 > ();
}

sal_Int32
CollatorWrapper::loadDefaultCollator (const lang::Locale& rLocale, sal_Int32 nOptions)
{
	try
	{
		if (mxInternationalCollator.is())
			return mxInternationalCollator->loadDefaultCollator (rLocale, nOptions);
	}
	catch (uno::RuntimeException& rRuntimeException)
	{
        (void)rRuntimeException;
		DBG_ERRORFILE ("CollatorWrapper: loadDefaultCollator failed");
	}

	return 0;
}

sal_Int32
CollatorWrapper::loadCollatorAlgorithm (const ::rtl::OUString& rAlgorithm,
		const lang::Locale& rLocale, sal_Int32 nOptions)
{
	try
	{
		if (mxInternationalCollator.is())
			return mxInternationalCollator->loadCollatorAlgorithm (
														rAlgorithm, rLocale, nOptions);
	}
	catch (uno::RuntimeException& rRuntimeException)
	{
        (void)rRuntimeException;
		DBG_ERRORFILE ("CollatorWrapper: loadCollatorAlgorithm failed");
	}

	return 0;

}

void
CollatorWrapper::loadCollatorAlgorithmWithEndUserOption (
		const ::rtl::OUString& rAlgorithm,
		const lang::Locale& rLocale, const uno::Sequence< sal_Int32 >& rOption)
{
	try
	{
		if (mxInternationalCollator.is())
			mxInternationalCollator->loadCollatorAlgorithmWithEndUserOption (
														rAlgorithm, rLocale, rOption);
	}
	catch (uno::RuntimeException& rRuntimeException)
	{
        (void)rRuntimeException;
		DBG_ERRORFILE ("CollatorWrapper: loadCollatorAlgorithmWithEndUserOption failed");
	}
}
