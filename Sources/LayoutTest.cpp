
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

    LayoutManager layoutManager;

    LayoutHandle mainLayout = layoutManager.createLayout();
    mainLayout->sizeSpecIs( SizeSpec::absolute( rootSize ) );
    mainLayout->paddingIs( padding );
    mainLayout->childGapIs( childGap );

    LayoutHandle child0 = layoutManager.createLayout();
    child0->parentIs( mainLayout );
    child0->sizeSpecIs( SizeSpec::grow() );
    mainLayout->addChild( child0 );

    mainLayout->computeLayout();
    REQUIRE( doctest::Approx( rootSize ) == mainLayout->size() );

    SUBCASE( "single child" ) {
        mainLayout->computeLayout();
        const double childSize = child0->size();
        const double mainSize = mainLayout->size();
        CHECK( doctest::Approx( childSize ) == ( mainSize - 2 * padding ) );
    }
    SUBCASE( "absolute child" ) {
        const double child1Size = 10;
        LayoutHandle child1 = layoutManager.createLayout();
        child1->parentIs( mainLayout );
        child1->sizeSpecIs( SizeSpec::absolute( child1Size ) );
        mainLayout->addChild( child1 );

        mainLayout->computeLayout();
        CHECK( child0->size() ==
               doctest::Approx( rootSize - child1Size - 2 * padding - childGap ) );
        CHECK( child1->size() == doctest::Approx( child1Size ) );
    }
    SUBCASE( "multiple expanding children" ) {
        const int numChildren = 3;
        std::vector< LayoutHandle > childHandles = { child0 };
        for( int i=0; i<( numChildren - 1 ); ++i ) {
            LayoutHandle child = layoutManager.createLayout();
            childHandles.push_back( child );
            child->parentIs( mainLayout );
            child->sizeSpecIs( SizeSpec::grow() );
            mainLayout->addChild( child );
        }

        mainLayout->computeLayout();
        const double expectedChildSize =
            ( rootSize - 2 * padding - ( numChildren - 1 ) * childGap ) /
            static_cast< double >( numChildren );
        for( const LayoutHandle & child : childHandles ) {
            CHECK( child->size() == doctest::Approx( expectedChildSize ) );
        }
    }
}
