typedef shared_ptr< sf::Texture > TexturePtr;
typedef shared_ptr<sf::Font> FontPtr;

class SpriteComponent
{
public:

	SpriteComponent(GameObject* inGameObject);
	~SpriteComponent();


	void SetTexture(TexturePtr inTexture);
	virtual sf::Sprite& GetSprite();
	

    void SetVisibility(bool inVisible) { mIsVisible = inVisible; }
    bool IsVisible() const { return mIsVisible; }


protected:

	sf::Sprite m_sprite;

	//don't want circular reference...
	GameObject* mGameObject;
    bool        mIsVisible;
};

typedef shared_ptr< SpriteComponent >	SpriteComponentPtr;

