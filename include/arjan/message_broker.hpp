#pragma once

#include <type_traits>
#include <variant>
#include <memory>
#include <vector>

namespace arjan {

template< class, class, class = void >
struct has_call_operator_for_type : std::false_type { };

template< typename FunctionObject, typename Argument >
struct has_call_operator_for_type< FunctionObject, Argument, std::void_t< decltype( std::declval< FunctionObject >()( std::declval< Argument >() ) ) > > : std::true_type { };

template< typename FunctionObject, typename Argument >
static constexpr auto has_call_operator_for_type_v = has_call_operator_for_type< FunctionObject, Argument >::value;

template < typename T >
struct subscription_base
{
	virtual constexpr ~subscription_base() = default;
	virtual constexpr void visit( T& ) = 0;
};

template < typename FunctionObject, typename MessageType >
struct subscription : subscription_base< MessageType >
{
	template < typename L >
	constexpr explicit subscription( L &&l ) :
		callback( std::forward< L >( l ) ) {}

	constexpr void visit( MessageType &message ) final
	{
		std::visit(
			[this]< typename T >( T &m )
			{
				if constexpr ( has_call_operator_for_type_v< FunctionObject, T& > )
				{
					callback( m );
				}
			},
			message
		);
	}

	[[ no_unique_address ]] const FunctionObject callback;
};

template < typename ...Types >
struct message_broker
{
	using message_type = std::variant< Types... >;
	using subscription_type = std::shared_ptr< subscription_base< message_type > >;

	template < typename T >
	[[ nodiscard ]] constexpr subscription_type subscribe( T &&callback )
	{
		static_assert(
			( has_call_operator_for_type_v< T, Types& > || ... ),
			"supplied callback does not accept any of the message types"
		);

		new_registrations.emplace_back(
			std::make_shared< subscription< T, message_type > >(
				std::forward< T >( callback )
			)
		);

		return new_registrations.back();
	}

	constexpr void publish( message_type message )
	{
		size_t offset = 0;
		while ( new_registrations.size() || offset != registrations.size() )
		{
			std::move( new_registrations.begin(), new_registrations.end(), std::back_insert_iterator( registrations ) );
			new_registrations.clear();

			auto iterator = registrations.begin() + offset;
			for ( auto i = offset; i < registrations.size(); ++i )
			{
				if ( iterator->use_count() > 1 )
				{
					iterator->get()->visit( message );
					++iterator;
				}
				else
				{
					std::rotate( iterator, iterator + 1, registrations.end() );
				}
			}
			registrations.erase( iterator, registrations.end() );
			offset = registrations.size();
		}
	}

	std::vector< subscription_type > registrations;
	std::vector< subscription_type > new_registrations;
};

}