#pragma once
#include "MoveBroadcaster.h"
#include "RefCountable.h"
#include "ID_Ptr.hpp"

class ContentsEntity;

namespace ServerCore
{
	class IocpEvent;
	class Sector;

	/*--------------
		IocpCore
	---------------*/

	// IOCP에 등록 가능한 모든 오브젝트
	class IocpObject
		:public RefCountable
	{
	protected:
		IocpObject();
		virtual ~IocpObject();
	public:
		virtual HANDLE GetHandle()const noexcept { return nullptr; }
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept abstract;
		virtual const bool IsValid()const noexcept { return true; }
		template<typename T = IocpObject>
		S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
	};

	class IocpEntity
		:public IocpObject
	{
	public:
		IocpEntity(const uint16_t type_id) noexcept
			: m_objectCombineID{ CombineObjectID(type_id,IDGenerator::GenerateID()) }
		{}
		virtual ~IocpEntity()noexcept = default;
	public:
		const S_ptr<ContentsEntity>& GetContentsEntity()const noexcept { return m_pContentsEntity; }
		const uint64_t GetObjectID()const noexcept { return ServerCore::GetObjectID(m_objectCombineID); }
		const uint16_t GetObjectType()const noexcept { return ServerCore::GetObjectType(m_objectCombineID); }
		PacketSession* const IsSession()noexcept
		{
			return reinterpret_cast<PacketSession* const>(
				static_cast<const int64_t>(-!GetObjectType()) &
				reinterpret_cast<const int64_t>(this)
				);
		}
		const PacketSession* const IsSession()const noexcept
		{
			return reinterpret_cast<const PacketSession* const>(
				static_cast<const int64_t>(-!GetObjectType()) &
				reinterpret_cast<const int64_t>(this)
				);
		}
		template<typename T>
		T* const ObjectCast(const uint16_t type_id)const noexcept
		{
			return reinterpret_cast<T* const>(
				static_cast<const int64_t>(-!!(GetObjectType() == type_id)) &
				reinterpret_cast<const int64_t>(this)
				);
		}
		template<typename T = IocpEntity>
		S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
		void SetEntity(S_ptr<ContentsEntity> pEntity_)noexcept { m_pContentsEntity.swap(pEntity_); }
		const U_Pptr<MoveBroadcaster>& GetMoveBroadcaster()const noexcept { return m_broadCaster; }
		const ID_Ptr<Sector> GetCombinedSectorInfo()const noexcept { return m_CurrentSectorInfo.load(std::memory_order_acquire); }
		void SetSectorInfo(const uint16_t prev_sector_id, const Sector* const cur_sector)noexcept {
			m_CurrentSectorInfo.store(ID_Ptr<Sector>{ prev_sector_id, cur_sector }, std::memory_order_release);
		}
		const uint16_t GetPrevSectorID()const noexcept { GetCombinedSectorInfo().GetID(); }
		template <typename T = Sector>
		T* const GetCurSector()const noexcept { return GetCombinedSectorInfo().GetPtr(); }
	private:
		const uint64_t m_objectCombineID;
		const U_Pptr<MoveBroadcaster> m_broadCaster = MakePoolUnique<MoveBroadcaster>();
		std::atomic<ID_Ptr<Sector>> m_CurrentSectorInfo;
		S_ptr<ContentsEntity> m_pContentsEntity;
	};
}