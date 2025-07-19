#pragma once

#include <vector>

#include "SizeTypes.hpp"

using ComponentSize = Vector2;

class Component {
public:
    virtual ~Component() = default;

    virtual ComponentSize size() const = 0;
    virtual void draw( Vector2 at ) = 0;
};

class Text: public Component {
public:
    Text( const std::string & content );

    ComponentSize size() const override;
    void draw( Vector2 at ) override;

private:
    std::string _content;

    rl::Font _font = rl::GetFontDefault();
    double _fontSize = 12;
    double _textSpacing = 1;
    rl::Color _textColor = rl::BLACK;

    ComponentSize _size = ComponentSize( 0, 0 );
};

class VStack: public Component {
public:
    VStack();

    void addComponent( std::unique_ptr< Component > c );
    void spacingIs( double val ) { _spacing = val; }

    ComponentSize size() const override;
    void draw( Vector2 at ) override;

private:
    std::vector< std::unique_ptr< Component > > _components;
    double _spacing = 0;
};

class HStack: public Component {
public:
    HStack();

    void addComponent( std::unique_ptr< Component > c );
    void spacingIs( double val ) { _spacing = val; }

    ComponentSize size() const override;
    void draw( Vector2 at ) override;

private:
    std::vector< std::unique_ptr< Component > > _components;
    double _spacing = 0;
};
