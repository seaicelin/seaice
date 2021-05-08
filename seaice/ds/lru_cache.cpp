    LruCache(size_t max_size = 0, size_t elasticity = 0, CacheStatus* status = nullptr);
    ~LruCache();
   
   void set(const K& k, const V& v);
   V get(const K& k);
   bool get(const K& k, const V& v);
   bool del(const K& k);
   bool exist(const K& k);
   size_t size();
   bool empty();
   bool clear();
   void setMaxSize(const size_t& v);
   void setElasticity(const size_t& v);
   size_t getMaxSize() const;
   size_t getElasticity() const;
   size_t getMaxAllowSize() const;
   void setPruneCallback(prune_callback cb);
   std::string toStatusString() const;
   CacheStatus* getStatus() const;
   void setStatus(CacheStatus* v, bool owner = false);

protected:
	size_t prune();