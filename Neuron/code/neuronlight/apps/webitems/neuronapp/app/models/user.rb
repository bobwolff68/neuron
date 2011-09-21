class User < ActiveRecord::Base
  
  [:username, :password, :first_name, :last_name].each do |field|
    validates_presence_of field
  end

end
