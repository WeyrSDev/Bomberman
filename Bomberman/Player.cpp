#include "Player.h"



Player::Player()
	:m_bomb(nullptr),
	m_canBeDamaged(true)
{

}


Player::~Player()
{
}


void Player::SetAnimator(Animator& animator, size_t width, size_t height)
{
	float w = static_cast<float>(width);
	float h = static_cast<float>(height);

	float tileSize = 64; // HACK tmp only

	m_sprite.setOrigin(w / 2.f, h / 2.f);
	SetSize(w, h);
	
	m_animator = &animator;
	m_animator->SetSprite(m_sprite);
	m_sprite.setPosition(sf::Vector2f(3 * tileSize + w, 5 * tileSize + h));
}

void Player::SetRespawns(short hp)
{
	m_respawns=m_maxNumberOfRespawns = hp;
}

void Player::IncreaseRespawns(short val)
{
	m_respawns += val;
}

void Player::DecreaseRespawns(short val)
{
	m_respawns -= val;
}

short Player::GetRespawnsCount()
{
	return m_respawns;
}


void Player::OnMoveKeyPressed(int x, int y)
{
	const float speed = 120;
	SetVelocity(speed*x, speed*y);
}

void Player::SetAfterRespawnSafeTime(float value)
{
	m_respawnSafeTime = sf::seconds(value);
}

bool Player::HasBomb()
{
	return (m_bomb != nullptr && m_bomb->GetState() == Bomb::waitingForExplosion);
}

Bomb * Player::GetBomb()
{
	return m_bomb;
}

void Player::OnActionKeyPressed()
{
	if(m_bomb==nullptr)
	{
		m_bomb = new Bomb();
		m_bombAnimator = new Animator();
		m_bombAnimator->AddAnimationState("waitingForExplosion", *m_bombTextureAtlas, 0, m_bombTextureAtlas->GetCount() - 1);
		m_bomb->SetAnimator(*m_bombAnimator, m_bombTextureAtlas->GetCellSizeX(), m_bombTextureAtlas->GetCellSizeY());
		m_bomb->SetUpRay(m_bombRayTextureAtlas);
		m_bomb->SetDetonationTime(sf::seconds(3.f));
		m_bomb->SetRayOnScreenTime(sf::seconds(1));
		m_bomb->SetPosition(static_cast<int>(GetPositionX()), static_cast<int>(GetPositionY()));
		m_bomb->SetLevelPointer(level);
		level->SetTileAsBomb(GetPositionX() / TILE_SIZE, GetPositionY() / TILE_SIZE);
	}
}

void Player::SetUpBomb(TextureAtlas* atlasBomb, TextureAtlas* atlasRay)
{
	m_bombTextureAtlas = atlasBomb;
	m_bombRayTextureAtlas = atlasRay;
}


void Player::SetLevelPointer(Level * level)
{
	if (level == nullptr)
	{
		std::cerr << "Level pointer is a null!" << std::endl;
		exit(5);
	}
	this->level = level;
}


void Player::Update(const float & dt)
{
	m_animator->Animate(dt);

	if (movementX != 0 && movementY != 0)
	{
		movementX /= 1.41f;
		movementY /= 1.41f;
	}
	
	SetPositionX(GetPositionX() + movementX);
	SetPositionY(GetPositionY() + movementY);
	m_sprite.setPosition(GetPositionX(),GetPositionY());

	if (m_bomb != nullptr)
	{
		m_bomb->Update(dt);
		if (m_bomb->GetState() == Bomb::exploding)
		{
			level->DestroyTile(m_bomb->GetPositionInTileCoordinatesX(), m_bomb->GetPositionInTileCoordinatesY(), false);
		}
		if (m_bomb->GetState() == Bomb::exploded)
		{
			delete m_bomb;
			m_bomb = nullptr;
		}
	}
	if (m_respawns > 0 && m_respawns < m_maxNumberOfRespawns && (m_respawnClock.getElapsedTime() >= m_respawnSafeTime))
	{
		m_canBeDamaged = true;
	}
	else if (m_respawns <= 0 && m_isAlive)
	{
		endGame();
		m_isAlive = false;
	}
}

bool Player::IsTileCollidingInAxisX(size_t x)
{
	return (x == GetPositionInTilesCoordsX() || x==BodyInfo.leftBound || x==BodyInfo.rightBound);
}

bool Player::IsTileCollidingInAxisY(size_t y)
{
	return (y == GetPositionInTilesCoordsY() || y == BodyInfo.upBound || y == BodyInfo.downBound);
}


void Player::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	if (m_bomb != nullptr)
		target.draw(*m_bomb);

	target.draw(m_sprite);
}

void Player::endGame()
{
}


void Player::OnBombCollision()
{
	if (m_canBeDamaged)Respawn();
}

bool Player::isBombExplosion()
{
	return (m_bomb != nullptr && m_bomb->GetState() == Bomb::exploding);
}

void Player::SetColor(int i)
{
	if (i)
	{
		m_sprite.setColor(sf::Color::Cyan);
		return;
	}
}

Ray* Player::GetRay(unsigned int side)
{
	return m_bomb->GetRayPhysicalBody(side);
}

bool Player::GetIsAlive()
{
	return m_isAlive;
}

void Player::SetIsAlive(bool var)
{
	m_isAlive = var;
}

int Player::GetPositionInTilesCoordsX()
{
	return static_cast<int>(GetPositionX() / TILE_SIZE);
}

int Player::GetPositionInTilesCoordsY()
{
	return static_cast<int>(GetPositionY() / TILE_SIZE);
}

bool Player::IsCollidingWithBomb()
{
	return m_isCollidingWithBomb;
}

void Player::SetIsCollidingWithBomb(bool value)
{
	m_isCollidingWithBomb = value;
}

sf::Vector2i Player::GetSideBombCollidingWith()
{
	return m_sideBombCollidingWith;
}

void Player::SetSideBombCollidingWith(int x, int y)
{
	m_sideBombCollidingWith.x = x;
	m_sideBombCollidingWith.y = y;
	if (x || y)
	{
		m_bombCollidingWithLevelCoords.x = static_cast<int>(GetPositionX() / TILE_SIZE + x);

		m_bombCollidingWithLevelCoords.y = static_cast<int>(GetPositionY() / TILE_SIZE + y);
	}
	else
	{
		m_bombCollidingWithLevelCoords = sf::Vector2i(NULL,NULL);
	}
}

sf::Vector2i Player::GetBombCollidingWithCoordinates()
{
	return m_bombCollidingWithLevelCoords;
}

void Player::SetRespawnPosition(size_t x, size_t y)
{
	m_respawnPosition.x = x;
	m_respawnPosition.y = y;
}

void Player::Spawn()
{
	SetPositionX(static_cast<int>(m_respawnPosition.x*TILE_SIZE + TILE_SIZE / 2));
	SetPositionY(static_cast<int>(m_respawnPosition.y*TILE_SIZE + TILE_SIZE / 2));
}

void Player::Respawn()
{
	m_canBeDamaged = false;
	Spawn();
	m_respawns--;
	m_respawnClock.restart();
}
