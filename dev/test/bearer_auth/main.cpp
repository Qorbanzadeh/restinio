/*
	restinio
*/

#include <catch2/catch.hpp>

#include <restinio/helpers/http_field_parsers/bearer_auth.hpp>

#include <test/common/dummy_connection.hpp>

using namespace std::string_literals;

RESTINIO_NODISCARD
auto
make_dummy_endpoint()
{
	return restinio::endpoint_t{
			restinio::asio_ns::ip::address::from_string("127.0.0.1"),
			12345u
	};
}

TEST_CASE( "No Authorization field", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			restinio::http_request_header_t{},
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::no_auth_http_field == result.error() );
}

TEST_CASE( "Empty Authorization field", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			""s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::illegal_http_field_value == result.error() );
}

TEST_CASE( "Different encoding scheme", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"MyScheme param=value, anotherparam=anothervalue"s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::not_bearer_auth_scheme == result.error() );
}

TEST_CASE( "Wrong Bearer Authentification params", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer param=value, anotherparam=anothervalue"s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::invalid_bearer_auth_param == result.error() );
}

TEST_CASE( "No semicolon in id:secret pair", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer dXNlcnBhc3N3b3Jk"s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::invalid_id_secret_pair == result.error() );
}

TEST_CASE( "Empty id in id:secret pair", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer OnBhc3N3b3Jk"s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::empty_id == result.error() );
}

TEST_CASE( "Empty secret in id:secret pair", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer dXNlcjo="s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( !result );
	REQUIRE( extraction_error_t::empty_secret == result.error() );
}


TEST_CASE( "Valid Authorization field", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer dXNlcjoxMjM0"s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			restinio::http_field::authorization );

	REQUIRE( result );
	REQUIRE( "user" == result->id );
	REQUIRE( "1234" == result->secret );
}

TEST_CASE( "Valid X-My-Authorization field", "[bearer_auth]" )
{
	using namespace restinio::http_field_parsers::bearer_auth;

	restinio::http_request_header_t dummy_header{
			restinio::http_method_post(),
			"/"
	};
	dummy_header.set_field(
			restinio::http_field::authorization,
			"Bearer dXNlcjoxMjM0"s );
	dummy_header.set_field(
			"X-My-Authorization",
			"Bearer bXktdXNlcjpteS0xMjM0"s );

	auto req = std::make_shared< restinio::request_t >(
			restinio::request_id_t{1},
			std::move(dummy_header),
			"Body"s,
			dummy_connection_t::make(1u),
			make_dummy_endpoint() );

	const auto result = try_extract_params( *req,
			"x-my-authorization" );

	REQUIRE( result );
	REQUIRE( "my-user" == result->id );
	REQUIRE( "my-1234" == result->secret );
}
