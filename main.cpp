#include "arjan/message_broker.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace arjan;

struct event {};
struct mouse_event : event {};
struct keyboard_event : event {};

TEST_CASE( "message inheritance" )
{
	WHEN( "creating a broker with mouse and keyboard events" )
	{
		message_broker< mouse_event, keyboard_event > broker;
		WHEN( "adding a subscriber who listens to mouse_events" )
		{
			int receive_count = 0;
			auto mouse = broker.subscribe(
				[&]( mouse_event )
				{
					++receive_count;
				}
			);
			THEN( "sending a mouse_event" )
			{
				CHECK( receive_count == 0 );
				broker.publish( mouse_event{} );
				CHECK( receive_count == 1 );
			}
			THEN( "sending a keyboard_event" )
			{
				CHECK( receive_count == 0 );
				broker.publish( keyboard_event{} );
				CHECK( receive_count == 0 );
			}
		}
		WHEN( "adding a subscriber for generic events" )
		{
			int receive_count = 0;
			auto generic = broker.subscribe(
				[&]( event )
				{
					++receive_count;
				}
			);
			THEN( "sending a mouse_event" )
			{
				CHECK( receive_count == 0 );
				broker.publish( mouse_event{} );
				CHECK( receive_count == 1 );
			}
			THEN( "sending a keyboard_event" )
			{
				CHECK( receive_count == 0 );
				broker.publish( keyboard_event{} );
				CHECK( receive_count == 1 );
			}
		}
	}
}

TEST_CASE( "changing messages" )
{
	WHEN( "creating a broker for integers" )
	{
		message_broker< int > broker;
		auto a = broker.subscribe( 
			[]( int &i )
			{
				i *= 3;
			}
		);
		auto b = broker.subscribe( 
			[]( int &i )
			{
				i += 1;
			}
		);
		auto c = broker.subscribe( 
			[]( int &i )
			{
				i *= 2;
			}
		);
		int result = 0;
		auto d = broker.subscribe( 
			[&result]( int i )
			{
				result = i;
			}
		);
		broker.publish( 1 );
		CHECK( result == ((1*3)+1)*2 );
		WHEN( "removing the second subscription, the relative order is preserved" )
		{
			b.reset();
			result = 0;
			broker.publish( 1 );
			CHECK( result == (1*3)*2 );
		}
	}
}

TEST_CASE( "dynamically add/remove subscriptions" )
{
	message_broker< int > broker;
	std::vector< message_broker< int >::subscription_type > subscriptions;
	std::function< void(int) > remove_sub = [&](int)
	{
		subscriptions.erase( subscriptions.begin() );
	};
	std::function< void(int&) > add_sub = [&]( int &i )
	{
		if ( ++i > 10 ) return;

		subscriptions.push_back(
			broker.subscribe( add_sub )
		);
	};
	int i = 0;
	add_sub( i );
	CHECK( subscriptions.size() == 1 );
	broker.publish( 0 );
	CHECK( subscriptions.size() == 11 );
	subscriptions.push_back( broker.subscribe( remove_sub ) );
	CHECK( subscriptions.size() == 12 );
	broker.publish( 11 );
	CHECK( subscriptions.size() == 11 );
	while ( subscriptions.size() )
	{
		broker.publish( 10 );
	}
}


TEST_CASE( "implicit conversions" )
{
	message_broker< int, float > broker;
	int value = 0;
	auto r = broker.subscribe(
		[&]( double i )
		{
			value += i;
		}
	);
	broker.publish( 10 );
	CHECK( value == 10 );
	r = broker.subscribe(
		[&]( uint8_t i )
		{
			value += i;
		}
	);
	broker.publish( 10 );
	CHECK( value == 20 );
}