module;

#include <algorithm>
#include <vector>
#include <string>

export module DeluGame.PlayerState;
export import DeluGame.Player;
import ECS;
import DeluEngine;

namespace Delu
{
	export struct PlayerStateParams
	{
		PlayerData data;
		std::vector<int> levelUpBreakpoints;
	};

	export class PlayerState : public DeluEngine::SceneSystem
	{
	private:
		PlayerData m_playerData;
		std::vector<int> m_levelUpBreakpoints;
		int m_currentPowerPoints = 0;
		std::shared_ptr<Player> m_player;
	public:
		PlayerState(const gsl::not_null<ECS::Scene*> scene, PlayerStateParams params) :
			DeluEngine::SceneSystem(scene),
			m_playerData(std::move(params.data)),
			m_levelUpBreakpoints(std::move(params.levelUpBreakpoints))
		{
			Respawn();
		}

	public:
		Player* Respawn()
		{
			if (m_player)
				m_player->DeleteObject(m_player);

			m_player = GetScene().NewObject<Delu::Player>(ECS::UserGameObjectInitializer{}, m_playerData).lock();

			return m_player.get();
		}

		void SetPowerPoints(int points)
		{
			m_currentPowerPoints = points;
			auto it = std::upper_bound(m_levelUpBreakpoints.begin(), m_levelUpBreakpoints.end(), m_currentPowerPoints);
			m_playerData.startingArmoryLevel = std::distance(m_levelUpBreakpoints.begin(), it);

			if (m_player)
			{
				m_player->SetArmoryLevel(m_playerData.startingArmoryLevel);
			}
		}
	};
}