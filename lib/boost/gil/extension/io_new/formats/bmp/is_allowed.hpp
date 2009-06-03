/*
    Copyright 2009 Christian Henning
    Use, modification and distribution are subject to the Boost Software License,
    Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).
*/

/*************************************************************************************************/

#ifndef BOOST_GIL_EXTENSION_IO_BMP_IO_IS_ALLOWED_HPP_INCLUDED
#define BOOST_GIL_EXTENSION_IO_BMP_IO_IS_ALLOWED_HPP_INCLUDED

////////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief
/// \author Christian Henning \n
///
/// \date 2008 \n
///
////////////////////////////////////////////////////////////////////////////////////////

namespace boost { namespace gil { namespace detail {

template< typename View >
bool is_allowed( const image_read_info< bmp_tag >& info
               , mpl::true_   // is read_and_no_convert
               )
{
    bmp_bits_per_pixel::type src_bits_per_pixel; 

    switch( info._bits_per_pixel )
    {
        case 1:
        case 4:
        case 8:
        {
            if(  info._header_size == bmp_win32_info_size
              && info._compression != ct_rle8 
              && info._compression != ct_rle4
              )
            {
                src_bits_per_pixel = 32;
            }
            else
            {
                src_bits_per_pixel = 24;
            }

            break;
        }

        case 15:
        case 16:
        {
            src_bits_per_pixel = 24;

            break;
        }

        case 24:
        case 32:
        {
            src_bits_per_pixel = info._bits_per_pixel;

            break;
        }
        default:
        {
            io_error( "Pixel size not supported." );
        }
    }

    typedef typename channel_traits< typename element_type< View::value_type >::type >::value_type channel_t;
    bmp_bits_per_pixel::type dst_bits_per_pixel = detail::unsigned_integral_num_bits< channel_t >::value
                                                * num_channels< View >::value;

    return ( dst_bits_per_pixel == src_bits_per_pixel );
}

template< typename View >
bool is_allowed( const image_read_info< bmp_tag >& info
               , mpl::false_  // is read_and_convert
               )
{
    return true;
}

} // namespace detail
} // namespace gil
} // namespace boost

#endif // BOOST_GIL_EXTENSION_IO_BMP_IO_IS_ALLOWED_HPP_INCLUDED
