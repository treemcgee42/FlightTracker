
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

TEST_CASE( "fit along axis" ) {
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

    LayoutHandle parent = layoutManager.createLayout();
    parent->sizeSpecIs( SizeSpec::fit() );
    parent->paddingIs( padding );
    parent->childGapIs( childGap );

    SUBCASE( "two absolute children" ) {
        double child0Size = 7;
        LayoutHandle child0 = layoutManager.createLayout();
        child0->sizeSpecIs( SizeSpec::absolute( child0Size ) );
        child0->parentIs( parent );
        parent->addChild( child0 );

        double child1Size = 8;
        LayoutHandle child1 = layoutManager.createLayout();
        child1->sizeSpecIs( SizeSpec::absolute( child1Size ) );
        child1->parentIs( parent );
        parent->addChild( child1 );

        parent->computeLayout();
        CHECK( parent->size() == doctest::Approx( child0Size + child1Size +
                                                  childGap + 2 * padding ) );
    }

    SUBCASE( "nested fit" ) {
        LayoutHandle child0 = layoutManager.createLayout();
        child0->sizeSpecIs( SizeSpec::fit() );
        child0->parentIs( parent );
        parent->addChild( child0 );

        double child00Size = 5;
        LayoutHandle child00 = layoutManager.createLayout();
        child00->sizeSpecIs( SizeSpec::absolute( child00Size ) );
        child00->parentIs( child0 );
        child0->addChild( child00 );

        parent->computeLayout();
        CHECK( doctest::Approx( child0->size() ) == child00Size );
        CHECK( doctest::Approx( parent->size() ) == child0->size() + 2 * padding );
    }
}

TEST_CASE( "shrink across axis" ) {
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

    LayoutHandle parent = layoutManager.createLayout();
    parent->sizeSpecIs( SizeSpec::shrinkAcrossAxis() );
    parent->paddingIs( padding );
    parent->childGapIs( childGap );

    SUBCASE( "two absolute children" ) {
        double child0Size = 7;
        LayoutHandle child0 = layoutManager.createLayout();
        child0->sizeSpecIs( SizeSpec::absolute( child0Size ) );
        child0->parentIs( parent );
        parent->addChild( child0 );

        double child1Size = 8;
        LayoutHandle child1 = layoutManager.createLayout();
        child1->sizeSpecIs( SizeSpec::absolute( child1Size ) );
        child1->parentIs( parent );
        parent->addChild( child1 );

        parent->computeLayout();
        CHECK( parent->size() == doctest::Approx( child1Size + 2 * padding ) );
    }

    SUBCASE( "nested fit" ) {
        LayoutHandle child0 = layoutManager.createLayout();
        child0->sizeSpecIs( SizeSpec::fit() );
        child0->parentIs( parent );
        parent->addChild( child0 );

        double child00Size = 5;
        LayoutHandle child00 = layoutManager.createLayout();
        child00->sizeSpecIs( SizeSpec::absolute( child00Size ) );
        child00->parentIs( child0 );
        child0->addChild( child00 );

        parent->computeLayout();
        CHECK( doctest::Approx( child0->size() ) == child00Size );
        CHECK( doctest::Approx( parent->size() ) == child0->size() + 2 * padding );
    }
}
