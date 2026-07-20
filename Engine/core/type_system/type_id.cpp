namespace _type_id {
	size_t nextTypeId()
	{
		static size_t id = 0;
		return id++;
	}
}