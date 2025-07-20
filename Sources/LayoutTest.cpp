
#include <doctest/doctest.h>

#include "Layout.hpp"

TEST_CASE("expanding") {
    const double rootSize = 80;
    double padding;
    SUBCASE( "" ) { padding = 0; }
    SUBCASE( "" ) { padding = 10; }
    CAPTURE( padding );
    double childGap;
    SUBCASE( "" ) { childGap = 0; }
    SUBCASE( "" ) { childGap = 1; }
    CAPTURE( childGap );

    Layout1d root;
    root.sizeSpecIs( SizeSpec::absolute( rootSize ) );
    root.paddingIs( padding );
    root.childGapIs( childGap );

    Layout1d child0;
    child0.parentIs( &root );
    child0.sizeSpecIs( SizeSpec::grow() );
    root.addChild( child0 );

    SUBCASE( "single child" ) {
        root.computeLayout();
        CHECK( doctest::Approx( root.child( 0 ).size() ) ==
               root.size() - 2 * padding );
    }

    SUBCASE( "absolute child" ) {
        const double child1Size = 10;
        Layout1d child1;
        child1.parentIs( &root );
        child1.sizeSpecIs( SizeSpec::absolute( child1Size ) );
        root.addChild( child1 );

        root.computeLayout();
        CHECK( root.child( 0 ).size() ==
               doctest::Approx( rootSize - child1Size - 2 * padding - childGap ) );
        CHECK( root.child( 1 ).size() == doctest::Approx( child1Size ) );
    }

    SUBCASE( "multiple expanding children" ) {
        const int numChildren = 3;
        for( int i=0; i<2; ++i ) {
            Layout1d child;
            child.parentIs( &root );
            child.sizeSpecIs( SizeSpec::grow() );
            root.addChild( child );
        }

        root.computeLayout();
        const double expectedChildSize =
            ( rootSize - 2 * padding - ( numChildren - 1 ) * childGap ) /
            static_cast< double >( numChildren );
        for( int i=0; i<3; ++i ) {
            CHECK( root.child( i ).size() == doctest::Approx( expectedChildSize ) );
        }
    }
}
