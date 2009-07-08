/*************************************************************************
 * Copyright (C) 2009 Tavian Barnes <tavianator@gmail.com>               *
 *                                                                       *
 * This file is part of The Dimension Library.                           *
 *                                                                       *
 * The Dimension Library is free software; you can redistribute it and/  *
 * or modify it under the terms of the GNU Lesser General Public License *
 * as published by the Free Software Foundation; either version 3 of the *
 * License, or (at your option) any later version.                       *
 *                                                                       *
 * The Dimension Library is distributed in the hope that it will be      *
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 * Lesser General Public License for more details.                       *
 *                                                                       *
 * You should have received a copy of the GNU Lesser General Public      *
 * License along with this program.  If not, see                         *
 * <http://www.gnu.org/licenses/>.                                       *
 *************************************************************************/

// dmnsn_progress* wrapper.

#ifndef DIMENSIONXX_PROGRESS_HPP
#define DIMENSIONXX_PROGRESS_HPP

#include <tr1/memory> // For tr1::shared_ptr
#include <list>

namespace Dimension
{
  // Base class for persisting objects
  class Persist_Base
  {
  public:
    virtual ~Persist_Base() = 0;

  protected:
    Persist_Base() { }

  private:
    // Copying prohibited
    Persist_Base(const Persist_Base&);
    Persist_Base& operator=(const Persist_Base&);
  };

  // Class for persisting objects
  template <typename T>
  class Persist : public Persist_Base
  {
  public:
    Persist(T* t) : m_t(t) { }
    virtual ~Persist() { delete m_t; }

    T* persisted() const { return m_t; }

  private:
    T* m_t;
  };

  // Class for persisting many objects
  class Persister
  {
  public:
    // Persister();
    // Persister(const Persister& persister);
    // ~Persister();

    // Persister& operator=(const Persister& persister);

    template <typename T>
    void persist(T* t);

    // Access the first persisted element
    template <typename T>
    Persist<T>& first();

  private:
    // Copy-assignment prohibited
    Persister& operator=(const Persister&);

    std::list<std::tr1::shared_ptr<Persist_Base> > m_persists;
  };

  // dmnsn_progress* wrapper class to represent an asynchronous worker thread
  class Progress
  {
  public:
    explicit Progress(dmnsn_progress* progress);
    Progress(dmnsn_progress* progress, const Persister& persister);
    // Progress(const Progress& progress);

    // Finishes the job without throwing
    ~Progress();

    double progress() const;
    void wait(double progress) const;

    void new_element(unsigned int total);
    void increment();
    void done();

    // Wait for job to finish, throwing if the job failed
    void finish();

    // Access the set of persisted objects
    Persister& persister();

    // Access the wrapped C object.
    dmnsn_progress*       dmnsn();
    const dmnsn_progress* dmnsn() const;

  private:
    // Copy assignment prohibited
    Progress& operator=(const Progress&);

    std::tr1::shared_ptr<dmnsn_progress*> m_progress;
    Persister m_persister;
  };

  template <typename T>
  void
  Persister::persist(T* t)
  {
    m_persists.push_back(std::tr1::shared_ptr<Persist_Base>(new Persist<T>(t)));
  }

  template <typename T>
  Persist<T>&
  Persister::first()
  {
    return dynamic_cast<Persist<T>&>(*m_persists.front());
  }
}

#endif /* DIMENSIONXX_PROGRESS_HPP */
