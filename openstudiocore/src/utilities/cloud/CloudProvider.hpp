/**********************************************************************
* Copyright (c) 2008-2013, Alliance for Sustainable Energy.  
*  All rights reserved.
*  
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License as published by the Free Software Foundation; either
*  version 2.1 of the License, or (at your option) any later version.
*  
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*  
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**********************************************************************/

#ifndef UTILITIES_CLOUD_CLOUDPROVIDER_HPP
#define UTILITIES_CLOUD_CLOUDPROVIDER_HPP

#include <utilities/UtilitiesAPI.hpp>
#include <utilities/core/Optional.hpp>
#include <utilities/core/UUID.hpp>
#include <utilities/core/Url.hpp>
#include <utilities/core/Logger.hpp>

#include <boost/shared_ptr.hpp>
#include <vector>

namespace openstudio{
  namespace detail {
    class CloudProvider_Impl;
  }

  /// CloudSession returns the information needed to identify and reconnect to compute nodes started by a previous CloudProvider.
  class UTILITIES_API CloudSession {
  public:
    CloudSession(const std::string& cloudProviderType, const std::string& sessionId, const boost::optional<Url>& serverUrl, const std::vector<Url>& workerUrls);
    std::string cloudProviderType() const;
    std::string sessionId() const;
    boost::optional<Url> serverUrl() const;
    void setServerUrl(const Url& serverUrl);
    void resetServerUrl();
    std::vector<Url> workerUrls() const;
    void addWorkerUrl(const Url& workerUrl);
  private:
    std::string m_cloudProviderType;
    std::string m_sessionId;
    boost::optional<Url> m_serverUrl;
    std::vector<Url> m_workerUrls;
  };

  /// CloudProvider is an abstract base class for classes that provide cloud resources.
  class UTILITIES_API CloudProvider {
  public:

    /** @name Constructor */
    //@{

    //@}
    /** @name Destructors */
    //@{

    /// virtual destructor
    virtual ~CloudProvider();

    //@}
    /** @name Class members */
    //@{
    
    /// returns the name of this type of cloud provider, e.g. 'AWSProvider'
    std::string type() const;
    
    /// returns true if this computer is connected to the internet
    /// blocking call, clears errors and warnings
    bool internetAvailable() const;

    /// returns true if the cloud service can be reached (e.g. ping)
    /// blocking call, clears errors and warnings
    bool serviceAvailable() const;

    /// returns true if the cloud service validates user credentials
    /// blocking call, clears errors and warnings
    bool validateCredentials() const;

    /// returns the current session
    /// blocking call
    CloudSession session() const;

    /// returns true if can connect to a previously started session
    /// blocking call, clears errors and warnings
    bool reconnect(const CloudSession& session);

    /// returns the ip address of the cloud server if it is started and running
    boost::optional<Url> serverUrl() const;

    /// returns true if the cloud server successfully begins to start the server node
    /// returns false if terminated
    /// non-blocking call, clears errors and warnings
    bool startServer();

    /// returns the ip address of all cloud workers that are started and running
    std::vector<Url> workerUrls() const;

    /// returns the number of workers to be requested
    unsigned numWorkers() const;

    /// returns true if the cloud server successfully begins to start all worker nodes
    /// returns false if terminated
    /// non-blocking call, clears errors and warnings
    bool startWorkers();

    /// returns true if the server and all workers are running
    bool running() const;

    /// returns true if the cloud server successfully begins to stop all nodes
    /// returns false if not running
    /// non-blocking call, clears errors and warnings
    bool terminate();

    /// returns true if terminate has been called
    bool terminated() const;

    /// returns errors generated by the last operation
    std::vector<std::string> errors() const;
    
    /// returns warnings generated by the last operation
    std::vector<std::string> warnings() const;

  //@}
  /** @name Type Casting */
  //@{

    template<typename T>
      boost::shared_ptr<T> getImpl() const
    {  return boost::dynamic_pointer_cast<T>(m_impl); }

    /// cast to type T, can throw std::bad_cast
    template<typename T>
    T cast() const{
      boost::shared_ptr<typename T::ImplType> impl = this->getImpl<typename T::ImplType>();
      if (!impl){
        throw(std::bad_cast());
      }
      return T(impl);
    }

    /// cast to optional of type T
    template<typename T>
    boost::optional<T> optionalCast() const{
      boost::optional<T> result;
      boost::shared_ptr<typename T::ImplType> impl = this->getImpl<typename T::ImplType>();
      if (impl){
        result = T(impl);
      }
      return result;
    }

  //@}

  protected:

    CloudProvider(const boost::shared_ptr<detail::CloudProvider_Impl>& impl);

  private:

    boost::shared_ptr<detail::CloudProvider_Impl> m_impl;
    
    // configure logging
    REGISTER_LOGGER("utilities.cloud.CloudProvider");
  };

} // openstudio

#endif // UTILITIES_CLOUD_CLOUDPROVIDER_HPP
