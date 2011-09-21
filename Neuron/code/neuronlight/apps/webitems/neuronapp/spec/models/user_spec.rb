require 'spec_helper'

describe User do

  context "validations" do

    %w[username password first_name last_name].each do |field|
      it { should validate_presence_of field }
    end

  end
  
end
