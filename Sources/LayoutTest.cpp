
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

    LayoutHandle mainLayoutHandle = layoutManager.createLayout();
    {
        Layout & mainLayout = mainLayoutHandle.getLayoutMut();
        mainLayout.sizeSpecIs( SizeSpec::absolute( rootSize ) );
        mainLayout.paddingIs( padding );
        mainLayout.childGapIs( childGap );
    }

    LayoutHandle child0Handle = layoutManager.createLayout();
    {
        Layout & child0 = child0Handle.getLayoutMut();
        child0.parentIs( mainLayoutHandle );
        child0.sizeSpecIs( SizeSpec::grow() );
    }
    mainLayoutHandle.getLayoutMut().addChild( child0Handle );

    mainLayoutHandle.getLayoutMut().computeLayout();
    REQUIRE( doctest::Approx( rootSize ) ==
             mainLayoutHandle.getLayoutConst().size() );

    SUBCASE( "single child" ) {
        mainLayoutHandle.getLayoutMut().computeLayout();
        const double childSize = child0Handle.getLayoutConst().size();
        const double mainSize =
            mainLayoutHandle.getLayoutConst().size();
        CHECK( doctest::Approx( childSize ) ==
               ( mainSize - 2 * padding ) );
    }
    SUBCASE( "absolute child" ) {
        const double child1Size = 10;
        LayoutHandle child1Handle = layoutManager.createLayout();
        {
            Layout & child1 = child1Handle.getLayoutMut();
            child1.parentIs( mainLayoutHandle );
            child1.sizeSpecIs( SizeSpec::absolute( child1Size ) );
            mainLayoutHandle.getLayoutMut().addChild( child1Handle );
        }

        mainLayoutHandle.getLayoutMut().computeLayout();
        CHECK( child0Handle.getLayoutConst().size() ==
               doctest::Approx( rootSize - child1Size - 2 * padding - childGap ) );
        CHECK( child1Handle.getLayoutConst().size() == doctest::Approx( child1Size ) );
    }
    SUBCASE( "multiple expanding children" ) {
        const int numChildren = 3;
        std::vector< LayoutHandle > childHandles = { child0Handle };
        for( int i=0; i<( numChildren - 1 ); ++i ) {
            LayoutHandle childHandle = layoutManager.createLayout();
            childHandles.push_back( childHandle );
            Layout & child = childHandle.getLayoutMut();
            child.parentIs( mainLayoutHandle );
            child.sizeSpecIs( SizeSpec::grow() );
            mainLayoutHandle.getLayoutMut().addChild( childHandle );
        }

        mainLayoutHandle.getLayoutMut().computeLayout();
        const double expectedChildSize =
            ( rootSize - 2 * padding - ( numChildren - 1 ) * childGap ) /
            static_cast< double >( numChildren );
        for( const LayoutHandle & childHandle : childHandles ) {
            CHECK( childHandle.getLayoutConst().size() ==
                   doctest::Approx( expectedChildSize ) );
        }
    }
}
