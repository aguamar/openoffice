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



#include "ShapeContextHandler.hxx"
#include "oox/vml/vmldrawingfragment.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"

namespace oox { namespace shape {

using namespace ::com::sun::star;
using namespace core;
using namespace drawingml;

::rtl::OUString SAL_CALL ShapeContextHandler_getImplementationName()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.ShapeContextHandler" );
}

uno::Sequence< ::rtl::OUString > SAL_CALL
ShapeContextHandler_getSupportedServiceNames()
{
    uno::Sequence< ::rtl::OUString > s(1);
    s[0] = CREATE_OUSTRING( "com.sun.star.xml.sax.FastShapeContextHandler" );
    return s;
}

uno::Reference< uno::XInterface > SAL_CALL
ShapeContextHandler_createInstance( const uno::Reference< uno::XComponentContext > & context)
        SAL_THROW((uno::Exception))
{
    return static_cast< ::cppu::OWeakObject* >( new ShapeContextHandler(context) );
}


ShapeContextHandler::ShapeContextHandler
(uno::Reference< uno::XComponentContext > const & context) :
mnStartToken(0), m_xContext(context)
{
    try
    {
        mxFilterBase.set( new ShapeFilterBase(context) );
    }
    catch( uno::Exception& )
    {
    }
}

ShapeContextHandler::~ShapeContextHandler()
{
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getGraphicShapeContext(::sal_Int32 Element )
{
    if (! mxGraphicShapeContext.is())
    {
        FragmentHandlerRef rFragmentHandler
            (new ShapeFragmentHandler(*mxFilterBase, msRelationFragmentPath));
        ShapePtr pMasterShape;

        switch (Element & 0xffff)
        {
            case XML_graphic:
                mpShape.reset(new Shape("com.sun.star.drawing.GraphicObjectShape" ));
                mxGraphicShapeContext.set
                (new GraphicalObjectFrameContext(*rFragmentHandler, pMasterShape, mpShape, true));
                break;
            case XML_pic:
                mpShape.reset(new Shape("com.sun.star.drawing.GraphicObjectShape" ));
                mxGraphicShapeContext.set
                (new GraphicShapeContext(*rFragmentHandler, pMasterShape, mpShape));
                break;
            default:
                break;
        }
    }

    return mxGraphicShapeContext;
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getDrawingShapeContext()
{
    if (!mxDrawingFragmentHandler.is())
    {
        mpDrawing.reset( new oox::vml::Drawing( *mxFilterBase, mxDrawPage, oox::vml::VMLDRAWING_WORD ) );
        mxDrawingFragmentHandler.set
          (dynamic_cast<ContextHandler *>
           (new oox::vml::DrawingFragment
            ( *mxFilterBase, msRelationFragmentPath, *mpDrawing )));
    }

    return mxDrawingFragmentHandler;
}

uno::Reference<xml::sax::XFastContextHandler>
ShapeContextHandler::getContextHandler()
{
    uno::Reference<xml::sax::XFastContextHandler> xResult;

    switch (getNamespace( mnStartToken ))
    {
        case NMSP_doc:
        case NMSP_vml:
            xResult.set(getDrawingShapeContext());
            break;
        default:
            xResult.set(getGraphicShapeContext(mnStartToken));
            break;
    }

    return xResult;
}

// ::com::sun::star::xml::sax::XFastContextHandler:
void SAL_CALL ShapeContextHandler::startFastElement
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    static const ::rtl::OUString sInputStream
        (RTL_CONSTASCII_USTRINGPARAM ("InputStream"));

    uno::Sequence<beans::PropertyValue> aSeq(1);
    aSeq[0].Name = sInputStream;
    aSeq[0].Value <<= mxInputStream;
    mxFilterBase->filter(aSeq);

    mpThemePtr.reset(new Theme());

    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->startFastElement(Element, Attribs);
}

void SAL_CALL ShapeContextHandler::startUnknownElement
(const ::rtl::OUString & Namespace, const ::rtl::OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->startUnknownElement(Namespace, Name, Attribs);
}

void SAL_CALL ShapeContextHandler::endFastElement(::sal_Int32 Element)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->endFastElement(Element);
}

void SAL_CALL ShapeContextHandler::endUnknownElement
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->endUnknownElement(Namespace, Name);
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
ShapeContextHandler::createFastChildContext
(::sal_Int32 Element,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference< xml::sax::XFastContextHandler > xResult;
    uno::Reference< xml::sax::XFastContextHandler > xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xResult.set(xContextHandler->createFastChildContext
                    (Element, Attribs));

    return xResult;
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
ShapeContextHandler::createUnknownChildContext
(const ::rtl::OUString & Namespace,
 const ::rtl::OUString & Name,
 const uno::Reference< xml::sax::XFastAttributeList > & Attribs)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        return xContextHandler->createUnknownChildContext
            (Namespace, Name, Attribs);

    return uno::Reference< xml::sax::XFastContextHandler >();
}

void SAL_CALL ShapeContextHandler::characters(const ::rtl::OUString & aChars)
    throw (uno::RuntimeException, xml::sax::SAXException)
{
    uno::Reference<XFastContextHandler> xContextHandler(getContextHandler());

    if (xContextHandler.is())
        xContextHandler->characters(aChars);
}

// ::com::sun::star::xml::sax::XFastShapeContextHandler:
uno::Reference< drawing::XShape > SAL_CALL
ShapeContextHandler::getShape() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xResult;
    uno::Reference< drawing::XShapes > xShapes( mxDrawPage, uno::UNO_QUERY );

    if (mxFilterBase.is() && xShapes.is())
    {
        if (mpDrawing.get() != NULL)
        {
            mpDrawing->finalizeFragmentImport();
            if( const ::oox::vml::ShapeBase* pShape = mpDrawing->getShapes().getFirstShape() )
                xResult = pShape->convertAndInsert( xShapes );
        }
        else if (mpShape.get() != NULL)
        {
            mpShape->addShape(*mxFilterBase, mpThemePtr.get(), xShapes);
			xResult.set(mpShape->getXShape());
            mxGraphicShapeContext.clear( );
		}
    }

    return xResult;
}

css::uno::Reference< css::drawing::XDrawPage > SAL_CALL
ShapeContextHandler::getDrawPage() throw (css::uno::RuntimeException)
{
    return mxDrawPage;
}

void SAL_CALL ShapeContextHandler::setDrawPage
(const css::uno::Reference< css::drawing::XDrawPage > & the_value)
    throw (css::uno::RuntimeException)
{
    mxDrawPage = the_value;
}

css::uno::Reference< css::frame::XModel > SAL_CALL
ShapeContextHandler::getModel() throw (css::uno::RuntimeException)
{
    if( !mxFilterBase.is() )
        throw uno::RuntimeException();
    return mxFilterBase->getModel();
}

void SAL_CALL ShapeContextHandler::setModel
(const css::uno::Reference< css::frame::XModel > & the_value)
    throw (css::uno::RuntimeException)
{
    if( !mxFilterBase.is() )
        throw uno::RuntimeException();
    uno::Reference<lang::XComponent> xComp(the_value, uno::UNO_QUERY_THROW);
    mxFilterBase->setTargetDocument(xComp);
}

uno::Reference< io::XInputStream > SAL_CALL
ShapeContextHandler::getInputStream() throw (uno::RuntimeException)
{
    return mxInputStream;
}

void SAL_CALL ShapeContextHandler::setInputStream
(const uno::Reference< io::XInputStream > & the_value)
    throw (uno::RuntimeException)
{
    mxInputStream = the_value;
}

::rtl::OUString SAL_CALL ShapeContextHandler::getRelationFragmentPath()
    throw (uno::RuntimeException)
{
    return msRelationFragmentPath;
}

void SAL_CALL ShapeContextHandler::setRelationFragmentPath
(const ::rtl::OUString & the_value)
    throw (uno::RuntimeException)
{
    msRelationFragmentPath = the_value;
}

::sal_Int32 SAL_CALL ShapeContextHandler::getStartToken() throw (::com::sun::star::uno::RuntimeException)
{
    return mnStartToken;
}

void SAL_CALL ShapeContextHandler::setStartToken( ::sal_Int32 _starttoken ) throw (::com::sun::star::uno::RuntimeException)
{
    mnStartToken = _starttoken;


}

::rtl::OUString ShapeContextHandler::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return ShapeContextHandler_getImplementationName();
}

uno::Sequence< ::rtl::OUString > ShapeContextHandler::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return ShapeContextHandler_getSupportedServiceNames();
}

::sal_Bool SAL_CALL ShapeContextHandler::supportsService
(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSeq = getSupportedServiceNames();

    if (aSeq[0].equals(ServiceName))
        return sal_True;

    return sal_False;
}

}}
